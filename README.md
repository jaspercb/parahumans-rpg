## Synopsis

It is (will be) an isometric game of some sort.

## High-priority features
- More stats + conditions to modify them (subjective time, size, damage resistance)
- More conditions (regen)
- Upgrade collision system to support rectangles and rays
- Super basic abilities (bolt, aura, punch, teleport)

## Motivation

I thought it'd be neat to make an isometric superhero game. The main technical challenge that interests me is random but novel power generation and control, but approximately 0 work has been done on that so far.

## Installation

A version of ENTT (a header-only entity component system framework) is included in this repo, because ECS systems on github have a bad habit of changing unexpectedly. You'll need your own copy of SDL2. The Makefile is configured to compile on Windows with MinGW, but if I want to work with other people on this it'll probably need to be configured for Unix eventually.

## License

Everything under entt/ is licensed under the MIT license by Michele Caini.
All the other code is released under the MIT license by Jasper Chapman-Black.