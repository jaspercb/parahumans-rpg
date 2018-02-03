## Synopsis

It's an isometric game of some sort where you're a superhero or something.

## High-priority features
- More stats + conditions to modify them (subjective passage of time, size, damage resistance)
- More conditions (regen)
- Upgrade collision system to support rectangles and rays
- Super basic abilities (bolt, aura, punch, teleport)

## Motivation

I thought it'd be neat to make an isometric superhero game. The main technical challenge that interests me is random but novel power generation and control, but approximately 0 work has been done on that so far.

## Installation

A Vagrantfile is included if you want a machine this is guaranteed to work on.

You'll need your own copy of SDL2 to compile at all, and your own copy of Googletest to run the tests. A version of ENTT (a header-only entity component system framework) is included in this repo.

## License

Everything under entt/ is licensed under the MIT license by Michele Caini, and can be found at https://github.com/skypjack/entt.
All the other code is released under the MIT license by Jasper Chapman-Black.