# Source Plugin

Basic functions for running Play To Earn in source games

### Features
- IncrementWallet(client, amount)
Adds a specified amount to the player's wallet.
Useful for rewarding players for in-game actions like wins, objectives, etc.

- RegisterPlayer(client)
Registers a player in the database.
Should be called when a player joins the server for the first time.

- UpdateWallet(client, address)
Update player wallet based on address

- ShowMenu(client)
Displays an in-game menu showing the player's current balance and related options.
Provides a user-friendly interface for checking wallet status.

### Installation
Place the plugin file (play_to_earn.inc) into the following directory:
```
/addons/sourcemod/plugins/scripting/include
```

Deep instructions can be found in specific play to earn source game