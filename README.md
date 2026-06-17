# ⛏️ Ore Rush

> Red vs Blue — an **online 1v1, third-person** competitive mining game set in a procedurally generated mine. Dig ore, haul it to your vault, sabotage your rival; the first to fill the quota wins.

![Unreal Engine](https://img.shields.io/badge/Unreal%20Engine-5.7-313131?logo=unrealengine&logoColor=white)
![C++](https://img.shields.io/badge/C%2B%2B-Core%20Logic-00599C?logo=cplusplus&logoColor=white)
![Multiplayer](https://img.shields.io/badge/Multiplayer-1v1%20Listen%20Server-2ea44f)
![Status](https://img.shields.io/badge/status-in%20development-yellow)

---

## 📖 About

Ore Rush is a warm, competitive game where two miners (**Red** and **Blue**) dig ore in a procedurally generated mining valley and haul it back to their own depots. No killing — just **sabotage and theft**. The first to fill the target quota (e.g. 50 points) into their vault wins.

All core logic is written in **C++**; Blueprint is used only for presentation (mesh, animation, FX, UI). The architecture is **listen-server** and **server-authoritative**.

## ✨ Gameplay

- 🏃 **Movement & Dash** — third-person movement plus a short escape dash (server-authoritative).
- ⛏️ **Mining** — look at a vein and hold; ore goes into your wallet.
- 🎒 **Wallet** — limited capacity; you slow down when full and drop it when trapped.
- 🏦 **Vault & Score** — deposit your wallet at your depot; **score = vault**. The vault is open to raids.
- 💣 **Traps & Defenses** — built by spending ore (snare, mud, decoy, smoke / barrier, turret).
- ⚡ **Power-ups** — speed, capacity, mining speed, shield, reveal.
- 🗺️ **Procedural Map** — seed-based; both players see the same map.

## 💎 Ore Economy

| Ore | Value | Availability |
|-----|:-----:|--------------|
| 🪨 Iron | 1× | Unlimited — safe but slow |
| 🥇 Gold | 2× | Limited |
| 💎 Diamond | 5× | Very rare / limited |

## 🎮 Controls

| Action | Key |
|--------|-----|
| Move | `W` `A` `S` `D` |
| Look | `Mouse` |
| Jump | `Space` |
| Dash | `Left Shift` |
| Mine | `Left Mouse` (hold) |

## 🧱 Architecture

- **Engine:** Unreal Engine 5.7 · **Language:** C++ (core) + Blueprint (presentation)
- **Networking:** Listen-server, server-authoritative. Intent → `Server` RPC, FX → `Multicast`, state → `Replicated` / RepNotify.
- **Game module:** `OreRush`

```
Source/OreRush/
├─ Character/    # AOreRushCharacter — movement, Enhanced Input, dash, mining
├─ Components/   # UWalletComponent — wallet (carried ore)
├─ Ore/          # AOreVein — ore vein (3 types)
├─ Game/         # AOreRushGameMode, AOreRushGameState
├─ Player/       # AOreRushPlayerController, AOreRushPlayerState
└─ Core/         # OreRushTypes — ETeam, EOreType
```

## 🚀 Getting Started

-On development

## 🗺️ Roadmap

- [x] Character + input + dash + replication
- [x] Ore veins (3 types) + mining + wallet
- [x] Depot + vault + score
- [x] GameMode + quota + win condition
- [x] Economy + traps
- [x] Depot raiding + defenses
- [x] Power-ups
- [x] Procedural map generation
- [ ] UI (menu / HUD / tutorial / end screen)
- [ ] Cosmetics + SFX/VFX + balancing
- [ ] Packaging

## 📝 Changelog

- **16/06/2026** — Added the third-person miner character: movement, camera, jump, and server-authoritative **dash**.
- **16/06/2026** — Added the ore system: 3 vein types (iron / gold / diamond), look-and-mine mechanic, and the wallet (carrying + slow-down when full).
- **16/06/2026** — Added depots: deposit your wallet into your team vault to score (server-authoritative, replicated team scores).
- **17/06/2026** — Added match flow: automatic team assignment (host Red / joiner Blue), score quota, and win condition — the first team to fill the quota wins.
- **17/06/2026** — Mining now holds the miner in place; movement returns when you release or finish mining.
- **17/06/2026** — Added the economy and traps: spend wallet ore to build snare, mud, decoy, and smoke traps (server-authoritative, with placement cooldown and active limit). Snare stuns and drops your ore as a pickable, mud slows, decoy disguises as ore, smoke leaves a vision-blocking cloud.
- **17/06/2026** — Added depot raiding and defenses: a shared interaction interface routes look-and-hold for both mining and raiding; raid an enemy vault to steal score over time, with a "busted" reversal when the owner defends. Defenses (a blocking barrier wall and a stunning turret with projectile) and nearby-defense reinforcement reduce raid losses.
- **17/06/2026** — Added power-ups: timed pickups that respawn on a cooldown — speed, carry capacity, mining speed, and a shield that grants trap immunity (server-applied, replicated).
- **17/06/2026** — Added the procedural map generator: a per-match random seed deterministically scatters iron/gold/diamond veins, two depots, and power-ups with minimum spacing; the server spawns and replicates so both players share the same map, and each player spawns at their own depot.
- **18/06/2026** — Added the reveal power-up, configurable trap/defense lifetimes, per-player cosmetic color, and HUD data getters (selected trap, active count, buff timers).
- **18/06/2026** — Added Steam sessions (host / find / join with no IP typing, plus Steam invites) with a direct-IP/LAN fallback, and the main-menu framework (menu gamemode + controller that shows the menu and frees the cursor).
- **18/06/2026** — Added a lobby map with a ready zone: while both players wait they can mine for fun, and once both stand in the zone a replicated 5-second countdown sends everyone to the match map.
