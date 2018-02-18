## Synopsis

It's an isometric game of some sort where you're a superhero or something.

## Current features
- Move around
- Various conditions (burn, bleed, regen, invulnerability, vulnerability, speedup, alter subjective passage of time)
- Use abilities to apply above conditions and damage (basic projectile, fire-projectile-and-swap-location, blink, click-to-center-AOE effect)

## High-priority features
- 
- Upgrade collision system to support rays
- More stats + conditions to modify them (size?)

## Motivation

I thought it'd be neat to make an isometric superhero game. The main technical challenge that interests me is random but novel power generation and control, but that's still very WIP and indefinitely delayed due to engine work.

## Installation

A Vagrantfile is included if you want a machine this is guaranteed to work on, but you should move the Vagrantfile into the parent directory so the shared folder works properly.

You'll need your own copy of SDL2 to compile at all, and your own copy of Googletest to run the tests. Dependencies (right now, just ENTT, a header-only entity component system framework) are included as submodules.

## License

Everything not in a submodule is released under the MIT license by Jasper Chapman-Black.

Everything in a submodule has the license outlined in that submodule.