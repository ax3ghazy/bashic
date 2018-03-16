# BAsic SHell In C (BASHIC)

## Usage

```
gcc src.c -o bashic
./bashic
```
- `history` to display command history
- `!N` to execute the Nth command in history
- `!!` to execute the last command
- `exit` to terminate the shell

## Specs

- `cd`
- Colorized prompt
- Children are sent SIGINT (not SIGKILL) when the shell terminates
- Command history

## Scrots

![alt text](/bashic.png "Prompt")

## Later

- scripting?
- multithreading?
