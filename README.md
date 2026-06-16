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

1. Clone the repository.
2. Right-click `Ore_Rush.uproject` → **Generate Visual Studio project files**.
3. Open the solution and build as **Development Editor** (or just double-click the `.uproject`).
4. In the editor: **Play** → *Net Mode: Play As Listen Server*, *Number of Players: 2* to test with two players.

## 🗺️ Roadmap

- [x] Character + input + dash + replication
- [x] Ore veins (3 types) + mining + wallet
- [ ] Depot + vault + score
- [ ] GameMode + quota + win condition
- [ ] Economy + traps
- [ ] Depot raiding + defenses
- [ ] Power-ups
- [ ] Procedural map generation
- [ ] UI (menu / HUD / tutorial / end screen)
- [ ] Cosmetics + SFX/VFX + balancing
- [ ] Packaging

## 📝 Changelog

- **16/06/2026** — Added the third-person miner character: movement, camera, jump, and server-authoritative **dash**.
- **16/06/2026** — Added the ore system: 3 vein types (iron / gold / diamond), look-and-mine mechanic, and the wallet (carrying + slow-down when full).
