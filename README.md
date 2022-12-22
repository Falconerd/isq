# isq - game creation tools

## UI - isq_ui.h

The isq_ui library is a simple immediate-mode UI library that is working, but not feature rich.

The implementation is based off a series of articles by Ryan Fleury https://www.rfleury.com/p/ui-part-1-the-interaction-medium

It's based on the FlexBox system from HTML, but also handles absolute and relative positioning.

There is element wrapping working in both row and column types of flex.

There is text, but no text wrapping.

There is vertical scrolling, but not horizontal scrolling.

The user must supply a rendering function, see main.c for an example.

## Memory Allocators - isq_mem.h

Not currently usable.
