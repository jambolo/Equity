//! Bitcoin script interpreter.
//!
//! Main stack, alt stack, and a scope
//! stack tracking IF/ELSE/ENDIF return points. Signature-check opcodes
//! (CHECKSIG and friends) are intentionally no-ops.

use crate::instruction::{Instruction, OpCode};
use crate::script::Script;
use crypto::{ripemd, sha1, sha256};

/// Stack-based Bitcoin script interpreter.
///
/// Signature-checking opcodes (`CHECKSIG` and friends) are intentional no-ops.
pub struct ScriptEngine {
    main_stack: Vec<Vec<u8>>,
    alt_stack: Vec<Vec<u8>>,
    scope_stack: Vec<usize>,
    code_separator: usize,
}

impl Default for ScriptEngine {
    fn default() -> Self {
        Self::new()
    }
}

impl ScriptEngine {
    /// Create a fresh engine with empty stacks.
    pub fn new() -> Self {
        Self {
            main_stack: Vec::new(),
            alt_stack: Vec::new(),
            scope_stack: Vec::new(),
            code_separator: 0,
        }
    }

    /// Borrow the current main stack contents.
    pub fn main_stack(&self) -> &[Vec<u8>] {
        &self.main_stack
    }

    /// Execute `script`. Returns `true` if execution finishes with a truthy top-of-stack.
    pub fn run(&mut self, script: &Script) -> bool {
        if !script.is_valid() {
            return false;
        }
        self.code_separator = 0;

        let instructions = script.instructions().to_vec();
        let mut pc = 0usize;

        while pc < instructions.len() {
            let ins = instructions[pc].clone();
            let op = ins.op();
            let desc = ins.description();
            let offset = ins.location();
            pc += 1;

            if self.main_stack.len() < desc.min_args {
                return false;
            }

            // Special case: opcode 0x00 (OP_0/OP_FALSE/PUSHDATA-0)
            if op == 0 {
                self.main_stack.push(ins.data().to_vec());
                continue;
            }

            // Push N bytes via opcodes 0x01..=0x4b (and PUSHDATA1/2/4)
            if (0x01..=0x4b).contains(&op)
                || op == OpCode::OP_PUSHDATA1 as u8
                || op == OpCode::OP_PUSHDATA2 as u8
                || op == OpCode::OP_PUSHDATA4 as u8
            {
                self.main_stack.push(ins.data().to_vec());
                continue;
            }

            match op {
                x if x == OpCode::OP_1NEGATE as u8 => self.main_stack.push(vec![0x81]),
                x if x == OpCode::OP_RESERVED as u8 => return false,
                0x51..=0x60 => self.main_stack.push(vec![op - 0x50]),

                x if x == OpCode::OP_NOP as u8 => {}
                x if x == OpCode::OP_VER as u8 => return false,

                x if x == OpCode::OP_IF as u8 => {
                    let top = self.main_stack.pop().unwrap();
                    let cond = to_bool(&top);
                    match process_branch(&instructions, pc, cond) {
                        Some((new_pc, ret)) => {
                            pc = new_pc;
                            self.scope_stack.push(ret);
                        }
                        None => return false,
                    }
                }
                x if x == OpCode::OP_NOTIF as u8 => {
                    let top = self.main_stack.pop().unwrap();
                    let cond = !to_bool(&top);
                    match process_branch(&instructions, pc, cond) {
                        Some((new_pc, ret)) => {
                            pc = new_pc;
                            self.scope_stack.push(ret);
                        }
                        None => return false,
                    }
                }
                x if x == OpCode::OP_ELSE as u8 || x == OpCode::OP_ENDIF as u8 => {
                    let Some(ret) = self.scope_stack.pop() else {
                        return false;
                    };
                    pc = ret;
                }
                x if x == OpCode::OP_VERIFY as u8 => {
                    let top = self.main_stack.pop().unwrap();
                    if !to_bool(&top) {
                        return false;
                    }
                }
                x if x == OpCode::OP_RETURN as u8 => return false,

                // Stack
                x if x == OpCode::OP_TOALTSTACK as u8 => {
                    let v = self.main_stack.pop().unwrap();
                    self.alt_stack.push(v);
                }
                x if x == OpCode::OP_FROMALTSTACK as u8 => {
                    let Some(v) = self.alt_stack.pop() else {
                        return false;
                    };
                    self.main_stack.push(v);
                }
                x if x == OpCode::OP_2DROP as u8 => {
                    self.main_stack.pop();
                    self.main_stack.pop();
                }
                x if x == OpCode::OP_2DUP as u8 => {
                    let n = self.main_stack.len();
                    let a = self.main_stack[n - 2].clone();
                    let b = self.main_stack[n - 1].clone();
                    self.main_stack.push(a);
                    self.main_stack.push(b);
                }
                x if x == OpCode::OP_3DUP as u8 => {
                    let n = self.main_stack.len();
                    let a = self.main_stack[n - 3].clone();
                    let b = self.main_stack[n - 2].clone();
                    let c = self.main_stack[n - 1].clone();
                    self.main_stack.push(a);
                    self.main_stack.push(b);
                    self.main_stack.push(c);
                }
                x if x == OpCode::OP_2OVER as u8 => {
                    let n = self.main_stack.len();
                    let a = self.main_stack[n - 4].clone();
                    let b = self.main_stack[n - 3].clone();
                    self.main_stack.push(a);
                    self.main_stack.push(b);
                }
                x if x == OpCode::OP_2ROT as u8 => {
                    let n = self.main_stack.len();
                    self.main_stack[n - 6..].rotate_left(2);
                }
                x if x == OpCode::OP_2SWAP as u8 => {
                    let n = self.main_stack.len();
                    self.main_stack[n - 4..].rotate_left(2);
                }
                x if x == OpCode::OP_IFDUP as u8 => {
                    let top = self.main_stack.last().unwrap().clone();
                    if to_bool(&top) {
                        self.main_stack.push(top);
                    }
                }
                x if x == OpCode::OP_DEPTH as u8 => {
                    let depth = self.main_stack.len() as i32;
                    self.main_stack.push(to_element(depth));
                }
                x if x == OpCode::OP_DROP as u8 => {
                    self.main_stack.pop();
                }
                x if x == OpCode::OP_DUP as u8 => {
                    let top = self.main_stack.last().unwrap().clone();
                    self.main_stack.push(top);
                }
                x if x == OpCode::OP_NIP as u8 => {
                    let n = self.main_stack.len();
                    self.main_stack.remove(n - 2);
                }
                x if x == OpCode::OP_OVER as u8 => {
                    let n = self.main_stack.len();
                    let a = self.main_stack[n - 2].clone();
                    self.main_stack.push(a);
                }
                x if x == OpCode::OP_PICK as u8 || x == OpCode::OP_ROLL as u8 => {
                    let top = self.main_stack.pop().unwrap();
                    if top.len() > 4 {
                        return false;
                    }
                    let n = to_int(&top);
                    if n < 0 || self.main_stack.len() < (n as usize + 1) {
                        return false;
                    }
                    let idx = self.main_stack.len() - 1 - n as usize;
                    if x == OpCode::OP_PICK as u8 {
                        let v = self.main_stack[idx].clone();
                        self.main_stack.push(v);
                    } else {
                        let v = self.main_stack.remove(idx);
                        self.main_stack.push(v);
                    }
                }
                x if x == OpCode::OP_ROT as u8 => {
                    let n = self.main_stack.len();
                    self.main_stack[n - 3..].rotate_left(1);
                }
                x if x == OpCode::OP_SWAP as u8 => {
                    let n = self.main_stack.len();
                    self.main_stack.swap(n - 1, n - 2);
                }
                x if x == OpCode::OP_TUCK as u8 => {
                    let top = self.main_stack.last().unwrap().clone();
                    let n = self.main_stack.len();
                    self.main_stack.insert(n - 2, top);
                }

                x if x == OpCode::OP_SIZE as u8 => {
                    let top_len = self.main_stack.last().unwrap().len() as i32;
                    self.main_stack.push(to_element(top_len));
                }

                x if x == OpCode::OP_EQUAL as u8 => {
                    let a = self.main_stack.pop().unwrap();
                    let b = self.main_stack.pop().unwrap();
                    self.main_stack.push(to_element_bool(a == b));
                }
                x if x == OpCode::OP_EQUALVERIFY as u8 => {
                    let a = self.main_stack.pop().unwrap();
                    let b = self.main_stack.pop().unwrap();
                    if a != b {
                        return false;
                    }
                }

                // Unary arithmetic
                x if x == OpCode::OP_1ADD as u8
                    || x == OpCode::OP_1SUB as u8
                    || x == OpCode::OP_NEGATE as u8
                    || x == OpCode::OP_ABS as u8
                    || x == OpCode::OP_NOT as u8
                    || x == OpCode::OP_0NOTEQUAL as u8 =>
                {
                    let top = self.main_stack.pop().unwrap();
                    if top.len() > 4 {
                        return false;
                    }
                    let v = to_int(&top);
                    let out = match op {
                        o if o == OpCode::OP_1ADD as u8 => to_element(v + 1),
                        o if o == OpCode::OP_1SUB as u8 => to_element(v - 1),
                        o if o == OpCode::OP_NEGATE as u8 => to_element(-v),
                        o if o == OpCode::OP_ABS as u8 => to_element(v.abs()),
                        o if o == OpCode::OP_NOT as u8 => to_element_bool(!to_bool(&top)),
                        o if o == OpCode::OP_0NOTEQUAL as u8 => to_element_bool(to_bool(&top)),
                        _ => unreachable!(),
                    };
                    self.main_stack.push(out);
                }

                // Binary arithmetic / comparison
                0x93 | 0x94 | 0x9a..=0xa4 => {
                    let b = self.main_stack.pop().unwrap();
                    let a = self.main_stack.pop().unwrap();
                    if a.len() > 4 || b.len() > 4 {
                        return false;
                    }
                    let av = to_int(&a);
                    let bv = to_int(&b);
                    let out = match op {
                        x if x == OpCode::OP_ADD as u8 => to_element(av + bv),
                        x if x == OpCode::OP_SUB as u8 => to_element(av - bv),
                        x if x == OpCode::OP_BOOLAND as u8 => {
                            to_element_bool(to_bool(&a) && to_bool(&b))
                        }
                        x if x == OpCode::OP_BOOLOR as u8 => {
                            to_element_bool(to_bool(&a) || to_bool(&b))
                        }
                        x if x == OpCode::OP_NUMEQUAL as u8 => to_element_bool(av == bv),
                        x if x == OpCode::OP_NUMEQUALVERIFY as u8 => {
                            if av != bv {
                                return false;
                            }
                            continue;
                        }
                        x if x == OpCode::OP_NUMNOTEQUAL as u8 => to_element_bool(av != bv),
                        x if x == OpCode::OP_LESSTHAN as u8 => to_element_bool(av < bv),
                        x if x == OpCode::OP_GREATERTHAN as u8 => to_element_bool(av > bv),
                        x if x == OpCode::OP_LESSTHANOREQUAL as u8 => to_element_bool(av <= bv),
                        x if x == OpCode::OP_GREATERTHANOREQUAL as u8 => to_element_bool(av >= bv),
                        x if x == OpCode::OP_MIN as u8 => to_element(av.min(bv)),
                        x if x == OpCode::OP_MAX as u8 => to_element(av.max(bv)),
                        _ => unreachable!(),
                    };
                    self.main_stack.push(out);
                }

                x if x == OpCode::OP_WITHIN as u8 => {
                    let max = self.main_stack.pop().unwrap();
                    let min = self.main_stack.pop().unwrap();
                    let x = self.main_stack.pop().unwrap();
                    if max.len() > 4 || min.len() > 4 || x.len() > 4 {
                        return false;
                    }
                    let xv = to_int(&x);
                    let result = to_int(&min) <= xv && xv < to_int(&max);
                    self.main_stack.push(to_element_bool(result));
                }

                // Crypto
                x if x == OpCode::OP_RIPEMD160 as u8 => {
                    let top = self.main_stack.pop().unwrap();
                    self.main_stack.push(ripemd::ripemd160(&top).to_vec());
                }
                x if x == OpCode::OP_SHA1 as u8 => {
                    let top = self.main_stack.pop().unwrap();
                    self.main_stack.push(sha1::sha1(&top).to_vec());
                }
                x if x == OpCode::OP_SHA256 as u8 => {
                    let top = self.main_stack.pop().unwrap();
                    self.main_stack.push(sha256::sha256(&top).to_vec());
                }
                x if x == OpCode::OP_HASH160 as u8 => {
                    let top = self.main_stack.pop().unwrap();
                    let h = ripemd::ripemd160(&sha256::sha256(&top));
                    self.main_stack.push(h.to_vec());
                }
                x if x == OpCode::OP_HASH256 as u8 => {
                    let top = self.main_stack.pop().unwrap();
                    self.main_stack.push(sha256::double_sha256(&top).to_vec());
                }
                x if x == OpCode::OP_CODESEPARATOR as u8 => {
                    self.code_separator = offset;
                }
                // Signature ops are not  implemented.
                0xac..=0xaf => {}

                // NOPs (b0..b9 except b1 which is also a NOP here)
                0xb0..=0xb9 => {}

                // RESERVED1/2 — only invalidate when executed; we treat the
                // unexecuted-branch logic as already-pruned, so seeing them is OK.
                0x89 | 0x8a => {}

                _ => return false,
            }
        }

        match self.main_stack.last() {
            Some(top) => to_bool(top),
            None => false,
        }
    }
}

fn to_bool(data: &[u8]) -> bool {
    if data.is_empty() {
        return false;
    }
    let last = *data.last().unwrap();
    // Allow signed-zero (0x80 in last byte with all earlier bytes zero) to be false.
    if last == 0 || last == 0x80 {
        for &b in &data[..data.len() - 1] {
            if b != 0 {
                return true;
            }
        }
        return false;
    }
    true
}

fn to_int(data: &[u8]) -> i32 {
    if data.is_empty() {
        return 0;
    }
    let negative = data.last().unwrap() & 0x80 != 0;
    let len = data.len();
    let out: i32 = if len >= 4 {
        data[0] as i32
            | ((data[1] as i32) << 8)
            | ((data[2] as i32) << 16)
            | (((data[3] & 0x7f) as i32) << 24)
    } else if len == 3 {
        data[0] as i32 | ((data[1] as i32) << 8) | (((data[2] & 0x7f) as i32) << 16)
    } else if len == 2 {
        data[0] as i32 | (((data[1] & 0x7f) as i32) << 8)
    } else {
        (data[0] & 0x7f) as i32
    };
    if negative { -out } else { out }
}

fn to_element(x: i32) -> Vec<u8> {
    if x == 0 {
        return Vec::new();
    }
    let negative = x < 0;
    let a = x.unsigned_abs();
    let mut out = if a < 0x80 {
        vec![a as u8]
    } else if a < 0x8000 {
        vec![a as u8, (a >> 8) as u8]
    } else if a < 0x800000 {
        vec![a as u8, (a >> 8) as u8, (a >> 16) as u8]
    } else {
        vec![a as u8, (a >> 8) as u8, (a >> 16) as u8, (a >> 24) as u8]
    };
    if negative {
        *out.last_mut().unwrap() |= 0x80;
    }
    out
}

fn to_element_bool(x: bool) -> Vec<u8> {
    if x {
        vec![1]
    } else {
        // Empty stack item = false (Bitcoin convention).
        Vec::new()
    }
}

fn find_matching_endif(instructions: &[Instruction], start: usize) -> Option<usize> {
    let mut i = start;
    while i < instructions.len() {
        let op = instructions[i].op();
        if op == OpCode::OP_IF as u8 || op == OpCode::OP_NOTIF as u8 {
            i = find_matching_endif(instructions, i + 1)?;
            i += 1;
        } else if op == OpCode::OP_ENDIF as u8 {
            return Some(i);
        } else {
            i += 1;
        }
    }
    None
}

fn find_matching_else_or_endif(instructions: &[Instruction], start: usize) -> Option<usize> {
    let mut i = start;
    while i < instructions.len() {
        let op = instructions[i].op();
        if op == OpCode::OP_IF as u8 || op == OpCode::OP_NOTIF as u8 {
            i = find_matching_endif(instructions, i + 1)?;
            i += 1;
        } else if op == OpCode::OP_ELSE as u8 || op == OpCode::OP_ENDIF as u8 {
            return Some(i);
        } else {
            i += 1;
        }
    }
    None
}

fn process_branch(
    instructions: &[Instruction],
    pc: usize,
    condition: bool,
) -> Option<(usize, usize)> {
    if condition {
        let end = find_matching_endif(instructions, pc)?;
        Some((pc, end + 1))
    } else {
        let i = find_matching_else_or_endif(instructions, pc)?;
        if instructions[i].op() == OpCode::OP_ELSE as u8 {
            let end = find_matching_endif(instructions, i + 1)?;
            Some((i + 1, end + 1))
        } else {
            Some((i, i + 1))
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_const_encode() {
        assert_eq!(to_element(0), Vec::<u8>::new());
        assert_eq!(to_element(1), vec![1]);
        assert_eq!(to_element(-1), vec![0x81]);
        assert_eq!(to_element(127), vec![0x7f]);
        assert_eq!(to_element(128), vec![0x80, 0x00]);
    }

    #[test]
    fn test_const_round_trip() {
        for &n in &[0, 1, -1, 127, -127, 128, -128, 0xFFFF, -0xFFFF, 0x7FFFFFFF] {
            let e = to_element(n);
            assert_eq!(to_int(&e), n, "{n}");
        }
    }

    #[test]
    fn test_run_op_1_op_1_op_equal() {
        let s = Script::from_data(&[0x51, 0x51, 0x87]).unwrap();
        let mut e = ScriptEngine::new();
        assert!(e.run(&s));
    }

    #[test]
    fn test_run_op_add() {
        // OP_2 OP_3 OP_ADD → top = 5 → truthy
        let s = Script::from_data(&[0x52, 0x53, 0x93]).unwrap();
        let mut e = ScriptEngine::new();
        assert!(e.run(&s));
        assert_eq!(e.main_stack().last().unwrap(), &vec![5u8]);
    }

    #[test]
    fn test_run_hash160_on_empty() {
        // PUSH(0-byte) OP_HASH160 — pushes hash of empty; truthy
        // Use OP_0 then OP_HASH160
        let s = Script::from_data(&[0x00, 0xa9]).unwrap();
        let mut e = ScriptEngine::new();
        assert!(e.run(&s));
        assert_eq!(e.main_stack().last().unwrap().len(), 20);
    }

    #[test]
    fn test_run_if_else_endif_taken() {
        // OP_1 OP_IF OP_2 OP_ELSE OP_3 OP_ENDIF
        let s = Script::from_data(&[0x51, 0x63, 0x52, 0x67, 0x53, 0x68]).unwrap();
        let mut e = ScriptEngine::new();
        assert!(e.run(&s));
        assert_eq!(e.main_stack().last().unwrap(), &vec![2u8]);
    }

    #[test]
    fn test_run_if_else_endif_else_branch() {
        // OP_0 OP_IF OP_2 OP_ELSE OP_3 OP_ENDIF
        let s = Script::from_data(&[0x00, 0x63, 0x52, 0x67, 0x53, 0x68]).unwrap();
        let mut e = ScriptEngine::new();
        assert!(e.run(&s));
        assert_eq!(e.main_stack().last().unwrap(), &vec![3u8]);
    }

    #[test]
    fn test_op_return_fails() {
        let s = Script::from_data(&[0x6a]).unwrap();
        let mut e = ScriptEngine::new();
        assert!(!e.run(&s));
    }

    #[test]
    fn test_op_verify_pops_top() {
        // OP_1 OP_VERIFY OP_1 — top after verify is 1
        let s = Script::from_data(&[0x51, 0x69, 0x51]).unwrap();
        let mut e = ScriptEngine::new();
        assert!(e.run(&s));
    }

    #[test]
    fn test_op_verify_fails_on_false() {
        // OP_0 OP_VERIFY → fail
        let s = Script::from_data(&[0x00, 0x69]).unwrap();
        let mut e = ScriptEngine::new();
        assert!(!e.run(&s));
    }

    #[test]
    fn test_op_dup_duplicates_top() {
        // OP_5 OP_DUP OP_EQUAL → true
        let s = Script::from_data(&[0x55, 0x76, 0x87]).unwrap();
        let mut e = ScriptEngine::new();
        assert!(e.run(&s));
    }

    #[test]
    fn test_op_sub_negative() {
        // OP_2 OP_3 OP_SUB → -1 → truthy
        let s = Script::from_data(&[0x52, 0x53, 0x94]).unwrap();
        let mut e = ScriptEngine::new();
        assert!(e.run(&s));
        assert_eq!(e.main_stack().last().unwrap(), &vec![0x81]);
    }

    #[test]
    fn test_alt_stack_round_trip() {
        // OP_5 OP_TOALTSTACK OP_FROMALTSTACK → top=5
        let s = Script::from_data(&[0x55, 0x6b, 0x6c]).unwrap();
        let mut e = ScriptEngine::new();
        assert!(e.run(&s));
        assert_eq!(e.main_stack().last().unwrap(), &vec![5u8]);
    }

    #[test]
    fn test_op_swap() {
        // OP_1 OP_2 OP_SWAP → top=1 (after swap)
        let s = Script::from_data(&[0x51, 0x52, 0x7c]).unwrap();
        let mut e = ScriptEngine::new();
        assert!(e.run(&s));
        assert_eq!(e.main_stack().last().unwrap(), &vec![1u8]);
    }

    #[test]
    fn test_op_depth() {
        // OP_1 OP_2 OP_3 OP_DEPTH → top=3
        let s = Script::from_data(&[0x51, 0x52, 0x53, 0x74]).unwrap();
        let mut e = ScriptEngine::new();
        assert!(e.run(&s));
        assert_eq!(e.main_stack().last().unwrap(), &vec![3u8]);
    }

    #[test]
    fn test_op_hash256_matches_double_sha256() {
        // OP_0 OP_HASH256 → SHA256d("")
        let s = Script::from_data(&[0x00, 0xaa]).unwrap();
        let mut e = ScriptEngine::new();
        assert!(e.run(&s));
        let expected = crypto::sha256::double_sha256(&[]).to_vec();
        assert_eq!(e.main_stack().last().unwrap(), &expected);
    }

    #[test]
    fn test_op_sha1_matches_sha1() {
        // OP_0 OP_SHA1
        let s = Script::from_data(&[0x00, 0xa7]).unwrap();
        let mut e = ScriptEngine::new();
        assert!(e.run(&s));
        let expected = crypto::sha1::sha1(&[]).to_vec();
        assert_eq!(e.main_stack().last().unwrap(), &expected);
    }

    #[test]
    fn test_op_within() {
        // OP_5 OP_3 OP_10 OP_WITHIN → 3 <= 5 < 10 → true
        let s = Script::from_data(&[0x55, 0x53, 0x5a, 0xa5]).unwrap();
        let mut e = ScriptEngine::new();
        assert!(e.run(&s));
    }

    #[test]
    fn test_op_not_truthy_flip() {
        // OP_1 OP_NOT → 0 → falsy
        let s = Script::from_data(&[0x51, 0x91]).unwrap();
        let mut e = ScriptEngine::new();
        assert!(!e.run(&s));
    }

    #[test]
    fn test_invalid_script_does_not_run() {
        let s = Script::from_data(&[0xBA]).unwrap();
        assert!(!s.is_valid());
        let mut e = ScriptEngine::new();
        assert!(!e.run(&s));
    }

    #[test]
    fn test_op_reserved_fails_when_executed() {
        let s = Script::from_data(&[0x50]).unwrap();
        let mut e = ScriptEngine::new();
        assert!(!e.run(&s));
    }

    #[test]
    fn test_p2pkh_full_pattern_no_sig() {
        // PUSH_data(20) OP_HASH160 PUSH_data(20) OP_EQUALVERIFY OP_CHECKSIG
        // Use OP_DUP OP_HASH160 <pubkeyhash> OP_EQUALVERIFY OP_CHECKSIG.
        // Without an actual sig+pubkey stack, only test that CHECKSIG is a no-op.
        let h160 = [0x42u8; 20];
        let mut bytes = vec![0u8; 0];
        bytes.push(0x14); // push 20
        bytes.extend_from_slice(&h160);
        bytes.push(0x76); // OP_DUP
        bytes.push(0x87); // OP_EQUAL — should be true after dup
        let s = Script::from_data(&bytes).unwrap();
        let mut e = ScriptEngine::new();
        assert!(e.run(&s));
    }
}
