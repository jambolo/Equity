//! Small string utilities.

/// Shorten `input` to at most `max_size` characters by replacing the middle
/// with `"..."`.
///
/// When `max_size <= 3` the function falls back to plain truncation. When
/// `max_size >= input.len()` the input is returned unchanged.
///
/// Slices by byte index; only safe for ASCII input.
///
/// # Examples
///
/// ```
/// use utility::shorten_string;
///
/// assert_eq!(shorten_string("hello world", 7), "he...ld");
/// assert_eq!(shorten_string("short", 20), "short");
/// assert_eq!(shorten_string("abcdef", 3), "abc");
/// ```
pub fn shorten_string(input: &str, max_size: usize) -> String {
    const ELLIPSIS: &str = "...";
    const ELLIPSIS_LEN: usize = ELLIPSIS.len();

    if max_size >= input.len() {
        return input.to_string();
    }

    if max_size > ELLIPSIS_LEN {
        let prefix_size = (max_size - ELLIPSIS_LEN).div_ceil(2);
        let suffix_size = (max_size - ELLIPSIS_LEN) / 2;
        let prefix = &input[..prefix_size];
        let suffix = &input[input.len() - suffix_size..];
        return format!("{prefix}{ELLIPSIS}{suffix}");
    }

    input[..max_size].to_string()
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn shortens_long_input() {
        let out = shorten_string("This is a very long string that needs to be shortened", 20);
        assert!(out.len() <= 20);
        assert!(out.contains("..."));
    }

    #[test]
    fn no_change_when_within_max() {
        assert_eq!(shorten_string("short", 20), "short");
    }

    #[test]
    fn below_ellipsis_truncates() {
        assert_eq!(shorten_string("hello world", 2), "he");
    }

    #[test]
    fn max_size_zero() {
        assert_eq!(shorten_string("hello", 0), "");
    }

    #[test]
    fn equal_to_input_len() {
        assert_eq!(shorten_string("hello", 5), "hello");
    }

    #[test]
    fn just_above_ellipsis() {
        assert_eq!(shorten_string("abcdef", 4), "a...");
    }

    #[test]
    fn exact_ellipsis_size() {
        assert_eq!(shorten_string("abcdef", 3), "abc");
    }

    proptest::proptest! {
        #[test]
        fn never_exceeds_max(s in "[ -~]{0,200}", max in 0usize..50) {
            let out = shorten_string(&s, max);
            proptest::prop_assert!(out.len() <= s.len().max(max));
        }
    }
}
