# SaveRanks

> Persistent player ranks across sessions — your rank is saved on the server and restored next time you join.

[![Discord](https://img.shields.io/badge/Discord-Support%20Server-5865F2?logo=discord&logoColor=white)](https://discord.gg/S2jhjDCzV3)
![Server-side only](https://img.shields.io/badge/Installation-Server--side%20only-green)
![Arma Reforger](https://img.shields.io/badge/Game-Arma%20Reforger-red)

---

## Overview

**SaveRanks** is a server-side mod for Arma Reforger that makes player ranks persistent. Normally, every time you reconnect or the server restarts, your rank resets. With this mod installed, the server remembers each player's rank and automatically restores it when they spawn back in — so promotions earned in one session carry over to the next.

---

## How It Works

Player ranks are saved to a JSON file in the server's profile folder, keyed by each player's persistent backend ID.

- **On spawn** — the saved rank is reapplied automatically
- **On death / disconnect** — the current rank is snapshotted
- **Periodic autosave** — ranks are flushed at a configurable interval
- **Automatic pruning** — old entries are removed after a configurable number of days so the save file doesn't grow forever

---

## Installation

This mod is **server-side only** — no client install is required. Players do not need to install anything.

1. Add the mod to your server's mod list
2. Launch the server — a default config will be generated automatically on first run

---

## Configuration

The config file is located at:

```
<profile>/SaveRanks/config.json
```

| Option | Default | Description |
|---|---|---|
| `enabled` | `true` | Master on/off switch for the mod |
| `retentionDays` | `30` | How many days an inactive player's rank is kept. Set to `0` to keep forever |
| `persistOnDeath` | `true` | Snapshot rank when a player dies |
| `applyOnSpawn` | `true` | Restore saved rank when a player spawns |
| `autosaveSeconds` | `300` | How often (in seconds) ranks are periodically flushed to disk |

---

## Support

Join the Discord server for help, bug reports, or feedback:

**[discord.gg/S2jhjDCzV3](https://discord.gg/S2jhjDCzV3)**
