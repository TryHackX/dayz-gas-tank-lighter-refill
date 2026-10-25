# Gas Tank & Lighter Refill

Refill gas canisters at the gas tanks on the map and refill your petrol lighter - on every official map, fully configurable. For DayZ 1.30 (Badlands).

[Steam Workshop](https://steamcommunity.com/sharedfiles/filedetails/?id=3619846324) · DayZ 1.30 (Badlands) · author: TryHackX

## Support

If the mod is useful for you or your server, you can support further development:

| Currency | Network | Address |
|---|---|---|
| Monero (XMR) | Monero | `45hvee4Jv7qeAm6SrBzXb9YVjb8DkHtFtFh7qkDMxS9zYX3NRi1dV27MtSdVC5X8T1YVoiG8XFiJkh4p9UncqWGxHi4tiwk` |
| Bitcoin (BTC) | Bitcoin (on-chain) | `bc1qncavcek4kknpvykedxas8kxash9kdng990qed2` |
| Ethereum (ETH) | Ethereum (mainnet) | `0xa3d38d5Cf202598dd782C611e9F43f342C967cF5` |

## Features

- Hold a small, medium or large gas canister (or the new **Lighter gas** can) and look at a refill tank: *Fill with gas from the tank*.
- The Lighter gas can refills a Petrol Lighter (10 full refills per can by default).
- Refill tanks: large white tank and small gas tank on every map (Chernarus 590, Sakhal 50, Nasdara 27), large rusty tank on Livonia (91).
- Only a tank you look at directly works (not through walls), standing next to it.
- Optional limit of refills per tank until the server restart, with a "The gas tank is empty" message.
- Translated into 12 languages.

## Configuration

`<server profile>/GasTankRefillMod/config.json` (created on the first start, synced to players):

| Setting | Default | Meaning |
|---|---|---|
| `TankFillPercent` | 100 | a canister is filled up to this percentage |
| `TankFillTimeMultiplier` | 1.0 | filling time at a tank (0.1 - 10) |
| `TankReachDistance` | 2.0 | metres from the player's feet to the tank body (0.5 - 3.5) |
| `TankRefillsPerRestart` | 0 | canisters one tank fills until the restart, 0 = unlimited |
| `LighterRefillsPerCan` | 10 | full lighter refills per Lighter gas can |
| `LighterFillTimeMultiplier` | 1.0 | lighter refilling time (0.1 - 10) |
| `AllowedGasTanks` | white + small gas tank | refill tanks on every map (part of the model path or a class name) |
| `MapGasTanks` | Livonia: rusty tank | extra tanks for one map (`MapName` = world name) |

## Installation

- Add the mod to the `-mod=` parameter of the server **and** the client.
- Copy `keys/TryHackX_New.bikey` to the `keys` folder of the server.
- Add the entry from `gtr_types.xml` to `db/types.xml` (class `GasLighterRefillCan`).

## Building from source

1. DayZ Tools → **AddonBuilder**: source directory `addons/GasTankRefillMod`, addon prefix `GasTankRefillMod`, output `addons/`.
2. Sign the PBO with your own key (DayZ Tools → DSUtils / DSCreateKey + DSSignFile) and give servers your `.bikey`.
   The release on Steam Workshop is signed with `TryHackX_New`.

## Compatibility

- DayZ 1.30 (Badlands), tested on a dedicated server on Chernarus, Livonia, Sakhal and Nasdara and in game.
- No other mods required (Community Framework is no longer needed).

## License

© TryHackX. You are free to use this mod on your server. Please do not repack or reupload it without permission.
