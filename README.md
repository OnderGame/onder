# Onder

Yet another rogue-like 2D sandbox game

## Goals

- Make it in C++
- Document it well
- Have something presentable

## Concept

- The world is "underground" with multiple 2D layers.
- It is very dark.
- Some limited form of automation and construction
- Spooky skeletons
- Dig dig dig!
- Play with friends, of course :) (multiplayer)
- Extensible! Gamers *love* mods.

## Architecture

We need to finish fast, so focus on keeping it simple and debuggable.
This means primarily two things:

- Keep as little state as necessary ( -> less stuff to synchronize).
- Use containers *with* checks. "UB is faster" yadayada developer sanity matters more!

### Graphics

To reduce the amount of state necessary, we'll rely as much on ad-hoc *instancing* as possible:

- Upload one or more resources necessary for drawing a thing.
- At render time, specify any number of instances along with instance data.

### Physics

Physics can be very simple and only really requires two things:

- An arbitrarily large grid of (rounded?) square colliders
- Circle colliders for dynamic entities.

### World

The world consists of multiple, independent layers.

Each layer is arbitrarily large, so it needs to be sparse.

Each layer consists of tiles.
Each tile is grouped in 16x16 L0 chunks ( -> 256 chunks, a nice 8 bit number).
Each L0 chunk is grouped in 256x256 L1 chunks.
Each L1 chunk is grouped in 65536x65536 L2 chunks.
... up to `1<<32`, which sure is large enough?
Assuming 1 meter tiles, that's 4.3 million kilometeres,
or about 107 times the circumference of the earth!

If at the edges, we have three options:

- Impenetratable wall
- Instant death
- Wrap around

I'm in favor of wrap around. The earth is "round", after all.

To traverse, scatter holes all around to encourage exploration.

### Multiplayer

We should make the server authoritative to prevent cheating.
At the same time, we should give the client leeway to keep movement and actions instant.

Suggestion: make the player move instantly on the client, and sends a desired path to the server.
Server will move the player along the given path.
If the server cannot move the player to a checkpoint along a path in time,
the player is "rubberbanded" back.

Use UDP for multiplayer so we can simply drop stale packets
(unlike TCP, which forces in-order, "reliable" delivery).

The server decides which L0 chunks are visible to the client and which arent.
It is up to the client to request chunks, but it is up to the server whether to send
or reject those chunks.

Entity updates are managed entirely by the server, as those may enter and leave
the player's zone arbitrarily.

### Composition

To simplify (yes really) and make things faster,
we'll use a data-driven approach for everything:
No objects, but collections of arbitrary things wherever appropriate.

For example, bullets would be represented as:

```cpp
struct Bullet {
    Pos2 position;
    Vec2 direction;
};
```

and kept track of as:

```cpp
List<Bullet> bullets;
```

Of course, chunks matter, especially for the client,
so realistically it's more something like:

```cpp
ChunkMap<List<Bullet>> bullets;
```

## Source directory structure

`src/` is subdivided in:

- `collections`: `List`, `HashMap`, `OrdMap` ...
- `graphics` for rendering targets (window systems, framebuffers ...)
- `physics` for any (2D) physics
- `world` for chunk management
- `input` for player input devices (keyboard, mouse)
- `db` for persistent data (saves).
- `net` for multiplayer functions.
- more?

OS-specific code is put in `unix/`, `linux/`, `windows/` ... as appropriate
