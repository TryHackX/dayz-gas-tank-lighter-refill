Gas Tank and Lighter Refill 1.6.0 - server setup (DayZ 1.30 Badlands)

1. Copy keys\TryHackX_New.bikey to your server's "keys" folder.
2. Community Framework is no longer needed (older versions listed it as a dependency, the mod never used it).
3. Add the entry from gtr_types.xml to db\types.xml of your mission (GasLighterRefillCan, Industrial + Farm loot).
4. Start the server once - the config is created automatically:
   <server profile>\GasTankRefillMod\config.json
   The server sends the refill tanks and settings to every player on connect - restart the server after editing it.

How it works:
   Gas canister in hands (small / medium / large cooking gas canister or the Lighter gas can) + look at a refill tank
      -> "Fill with gas from the tank": the canister is filled to 100 %.
         Only a tank you look at directly works (not through a wall or another object), standing at most
         2 m from it (TankReachDistance).
         Time for an empty canister: Lighter gas 6 s, small 8 s, medium 10 s, large 14 s (less when it is partly full).
   Lighter gas can + Petrol Lighter (one in hands, the other on the ground or combined in the inventory)
      -> "Fill the lighter with gas": a full refill of an empty lighter uses 10 % of the can (10 refills per can).
   Action names and the item are translated (English, Polish, Czech, German, Russian, Hungarian, Italian,
   Spanish, French, Portuguese, Chinese, Japanese).

Default refill tanks:
   every map:         tanks\tank_small_white.p3d   (large white tank)
                      tanks\tank_small_gas.p3d     (small gas tank)
   Livonia (enoch):   tanks\tank_small_rusty.p3d   (large rusty tank - Livonia has no white or small gas tanks)
   Refill tanks found on the maps (DayZ 1.30): Chernarus 590, Livonia 91, Sakhal 50, Nasdara 27.

Config (config.json) - the default values keep the gameplay of the older versions (only the reach is 2 m):
   "ModEnabled"                1 = on, 0 = off
   "DebugLogging"              1 = extra lines in the server script log
   "TankFillPercent"           a canister is filled at a tank up to this percentage (1 - 100, default 100)
   "TankFillTimeMultiplier"    filling time at a tank: 1 = normal, 2 = twice as long (0.1 - 10, default 1)
   "TankReachDistance"         how far the player may stand from the tank, from the feet to the tank body in metres
                               (0.5 - 3.5, default 2)
   "TankRefillsPerRestart"     how many canisters one tank fills until the server restarts, 0 = unlimited (default 0).
                               Every started refill counts. An empty tank shows the player "The gas tank is empty".
   "LighterRefillsPerCan"      full refills of an empty lighter from one Lighter gas can (1 - 100, default 10)
   "LighterFillTimeMultiplier" lighter refilling time: 1 = normal (0.1 - 10, default 1)
   "AllowedGasTanks"           refill tanks on every map: a part of the model path (e.g. "tanks\\tank_small_rusty.p3d",
                               forward slashes work too) or a class name of a map object (e.g. "Land_Tank_Big")
   "MapGasTanks"               extra refill tanks used only on one map:
                               { "MapName": "enoch", "GasTanks": [ "tanks\\tank_small_rusty.p3d" ] }
                               MapName = world name: chernarusplus, enoch, sakhal, nasdara (or the world name of a community
                               map, the server log prints it: "Gas tanks on <world name>")
   Values out of range are clamped (a warning is printed in the server script log, the file is not changed).
   Other tank models you can add: tanks\tank_small_rusty.p3d (large rusty tank, also on Chernarus, Sakhal and Nasdara),
   tanks\tank_medium.p3d, Land_Tank_Big.

Upgrade from 1.5.x: your config.json is kept and upgraded automatically - the new settings are added with the default
values and "MapGasTanks" gets the rusty tank on Livonia. Your "AllowedGasTanks" list stays exactly as it was.
A config file with a syntax error is not overwritten - the server uses the default settings and prints the error
in the script log.
