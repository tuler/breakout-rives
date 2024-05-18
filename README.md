# Breakout

This is the classic [breakout](<https://en.wikipedia.org/wiki/Breakout_(video_game)>).

Submitted to the first [Rives Game Jam](https://itch.io/jam/rives1).

```shell
rivemu -workspace -exec riv-jit-c breakout.c
```

## Editor setup

Copy `riv.h` to project for better code completion.

```shell
rivemu -quiet -sdk -workspace -no-window -exec cp /usr/include/riv.h .
```
