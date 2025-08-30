# NeoPiagn

A rewrite of the original [Piagn](https://github.com/a8hnaia/piagn). Behaviour differs in two ways:

- Functions may only be 256 lines by 256 columns, the stack and the piles have 256 elements at most and function calls can only go 256 levels deep *for now*.
- The main function is marked with `@ {` similar to other functions instead of starting the line with `{` directly.
