#if defined play_to_earn
    #endinput
#endif
#define play_to_earn

#include <sdktools_stringtables>

public const char        DISCORD_INVITE[] = "discord.gg/vGHxVsXc4Q";

static const bool shouldDebug      = false;

//
// #REGION Increment Wallet
//

// Increment value to the player wallet
stock void        IncrementWallet(
           int client,
           char[] valueToIncrement,
           char[] valueToShow = "0",
           char[] reason      = " PTE, for Playing")
{
    if (!IsValidClient(client))
    {
        return;
    }

    int steamid = GetSteamAccountID(client);

    if (steamid == 0)
    {
        PrintToServer("[PTE-INC] Invalid client when incrementing wallet");
        return;
    }

    Database database = CreatePTEDatabaseConnection();
    if (database == null) return;

    char game[64];
    GetGameFolderName(game, sizeof(game));

    char query[256];
    Format(query, sizeof(query), "UPDATE `%s` SET value = value + ? WHERE uniqueid = ?", game);

    char        statementError[456];
    DBStatement statement = SQL_PrepareQuery(database, query, statementError, sizeof(statementError));

    SQL_BindParamString(statement, 0, valueToIncrement, true);
    SQL_BindParamInt(statement, 1, steamid);

    if (shouldDebug)
        PrintToServer("[PTE-INC] Query: UPDATE `%s` SET value = value + %s WHERE uniqueid = %d", game, valueToIncrement, steamid);

    if (!SQL_Execute(statement))
    {
        char databaseError[456];
        SQL_GetError(database, databaseError, sizeof(databaseError));
        PrintToServer("[PTE-INC] Database error: %s", databaseError);
        PrintToServer("[PTE-INC] Statement error: %s", statementError);
        PrintToChat(client, "[PTE] Cannot increment your wallet, contact server owner on: %s", DISCORD_INVITE);
    }
    else {
        PrintToChat(client, "[PTE] You received: %s%s", valueToShow, reason);
    }

    statement.Close();
    database.Close();
}
//
// #ENDREGION Increment Wallet
//

//
// #REGION Register Player
//

// Try to register the player, should be called before any function in this include
stock void RegisterPlayer(const int client)
{
    if (!IsValidClient(client))
    {
        return;
    }

    int steamid = GetSteamAccountID(client);

    if (steamid == 0)
    {
        PrintToServer("[PTE-INC] Invalid client when registering player");
        return;
    }

    Database database = CreatePTEDatabaseConnection();
    if (database == null) return;

    char game[64];
    GetGameFolderName(game, sizeof(game));

    char query[256];
    Format(query, sizeof(query), "INSERT INTO `%s` (uniqueid) VALUES (?)", game);

    char        statementError[456];
    DBStatement statement = SQL_PrepareQuery(database, query, statementError, sizeof(statementError));

    SQL_BindParamInt(statement, 0, steamid);

    if (shouldDebug)
        PrintToServer("[PTE-INC] Query: INSERT INTO `%s` (uniqueid) VALUES (%d)", game, steamid);

    if (!SQL_Execute(statement))
    {
        char databaseError[456];
        SQL_GetError(database, databaseError, sizeof(databaseError));

        if (StrContains(databaseError, "Duplicate entry", false) == -1)
        {
            PrintToServer("[PTE-INC] Database error: %s", databaseError);
            PrintToServer("[PTE-INC] Statement error: %s", statementError);
        }
    }

    statement.Close();
    database.Close();
}

//
// #ENDREGION Register Player
//

//
// #REGION Update Wallet
//

// Updates a player wallet based on player client
stock void UpdateWallet(const int client, const char walletaddress[256])
{
    if (!IsValidClient(client))
    {
        return;
    }

    int steamid = GetSteamAccountID(client);

    if (steamid == 0)
    {
        PrintToServer("[PTE-INC] Invalid client when registering player");
        return;
    }

    char from[64];
    GetGameFolderName(from, sizeof(from));

    Database database = CreatePTEDatabaseConnection();
    if (database == null) return;

    char game[64];
    GetGameFolderName(game, sizeof(game));

    char query[256];
    Format(query, sizeof(query), "UPDATE `%s` SET walletaddress = ? WHERE uniqueid = ?", game);

    char        statementError[456];
    DBStatement statement = SQL_PrepareQuery(database, query, statementError, sizeof(statementError));

    SQL_BindParamString(statement, 0, walletaddress, true);
    SQL_BindParamInt(statement, 1, steamid);

    if (shouldDebug)
        PrintToServer("[PTE-INC] Query: UPDATE `%s` SET walletaddress = %s WHERE uniqueid = %d", game, walletaddress, steamid);

    if (!SQL_Execute(statement))
    {
        char databaseError[456];
        SQL_GetError(database, databaseError, sizeof(databaseError));
        PrintToServer("[PTE-INC] Database error: %s", databaseError);
        PrintToServer("[PTE-INC] Statement error: %s", statementError);
        PrintToChat(client, "[PTE] Cannot register your address, contact server owner on: %s", DISCORD_INVITE);
    }
    else
    {
        PrintToServer("[PTE-INC] Wallet updated for SteamID %d", steamid);
        PrintToChat(client, "[PTE] Wallet changed!");
    }

    statement.Close();
    database.Close();
}
//
// #ENDREGION Update Wallet
//

//
// #REGION Show Menu
//

stock void ShowMenu(int client)
{
    if (!IsValidClient(client))
    {
        return;
    }

    int steamid = GetSteamAccountID(client);

    if (steamid == 0)
    {
        PrintToServer("[PTE-INC] Invalid client when showing menu");
        return;
    }

    Database database = CreatePTEDatabaseConnection();
    if (database == null) return;

    char game[64];
    GetGameFolderName(game, sizeof(game));

    char query[256];
    Format(query, sizeof(query), "SELECT value FROM `%s` WHERE uniqueid = ?", game);

    char        statementError[456];
    DBStatement statement = SQL_PrepareQuery(database, query, statementError, sizeof(statementError));

    SQL_BindParamInt(statement, 0, steamid);

    if (shouldDebug)
        PrintToServer("[PTE-INC] Query: SELECT value FROM `%s` WHERE uniqueid = %d", game, steamid);

    if (!SQL_Execute(statement))
    {
        char databaseError[456];
        SQL_GetError(database, databaseError, sizeof(databaseError));
        PrintToServer("[PTE-INC] Database error: %s", databaseError);
        PrintToServer("[PTE-INC] Statement error: %s", statementError);
    }

    char balance[128] = "0.0";

    if (SQL_HasResultSet(statement))
    {
        while (SQL_FetchRow(statement))
        {
            char value[128];
            SQL_FetchString(statement, 0, value, sizeof(value));

            if (shouldDebug)
                PrintToServer("[PTE-INC] Retrieved raw value for SteamID %d: %s", steamid, value);

            int len = strlen(value);

            if (len > 14)
            {
                int  newLen = len - 14;
                char temp[128];
                strcopy(temp, sizeof(temp), value);
                temp[newLen] = '\0';
                strcopy(value, sizeof(value), temp);
            }

            len = strlen(value);

            if (len == 1 || len == 2)
            {
                char temp[128];
                Format(temp, sizeof(temp), "0.%s", value);
                strcopy(value, sizeof(value), temp);
            }
            else {
                int  pos = len - 2;

                char temp[128];
                strcopy(temp, sizeof(temp), value);
                temp[pos] = '\0';

                char lastTwo[3];
                lastTwo[0] = value[pos];
                lastTwo[1] = value[pos + 1];
                lastTwo[2] = '\0';

                Format(value, sizeof(value), "%s.%s", temp, lastTwo);
            }

            if (shouldDebug)
                PrintToServer("[PTE-INC] Retrieved converted value for SteamID %d: %s", steamid, value);

            strcopy(balance, sizeof(balance), value);
        }
    }

    statement.Close();
    database.Close();

    Menu menu = new Menu(MenuHandler);
    menu.SetTitle("Your Current Balance: %s PTE\nYou can reopen this menu in chat: !menu", balance);
    menu.AddItem("1", "Request Payment");
    menu.AddItem("2", "Register Wallet");
    menu.AddItem("3", "View Steam ID");
    menu.Display(client, 8);
}

stock int MenuHandler(Menu menu, MenuAction action, int client, int param)
{
    if (action == MenuAction_Select)
    {
        char info[32];
        menu.GetItem(param, info, sizeof(info));

        if (StrEqual(info, "1"))
        {
            PrintToChat(client, "[PTE] Currently not supported on this system");
        }
        else if (StrEqual(info, "2"))
        {
            PrintToChat(client, "[PTE] You can register your wallet address in the chat, like: !wallet 0x123..., for more information you can check your discord: %s", DISCORD_INVITE);
            ShowMenu(client);
        }
        else if (StrEqual(info, "3"))
        {
            PrintToChat(client, "[PTE] Your steam id is: %d", GetSteamAccountID(client));
            ShowMenu(client);
        }
    }

    return 0;
}

//
// #ENDREGION Show Menu
//

//
// #REGION Utils
//

// Returns any array containing all "client" from online players
stock void GetOnlinePlayers(int[] onlinePlayers, int playerSize)
{
    int arrayIndex = 0;
    for (int i = 1; i < MaxClients; i += 1)
    {
        if (arrayIndex >= playerSize)
        {
            break;
        }

        int client = i;

        if (!IsValidClient(client))
        {
            continue;
        }

        onlinePlayers[arrayIndex] = client;
        arrayIndex++;
    }
}

// Returns a int number containing the count of online players
stock int GetOnlinePlayersCount()
{
    int count = 0;
    for (int i = 0; i < MaxClients; i += 1)
    {
        int client = i;

        if (!IsValidClient(client))
        {
            continue;
        }

        count++;
    }

    return count;
}

/**
 * Extracts the value of a given `key` from a simple JSON string `jsonStr`.
 *
 * @param jsonStr   JSON string (e.g. {"key": "value", "num": 123})
 * @param key       The key to extract the value from (e.g. "quantity")
 * @param out       Buffer where the extracted value will be stored
 * @param maxlen    Maximum size of the `out` buffer
 *
 * @return true if the key was found and value extracted, false otherwise
 */
stock bool ExtractJsonValue(const char[] jsonStr, const char[] key, char[] out, int maxlen)
{
    int keyPos = StrContains(jsonStr, key);
    if (keyPos == -1)
    {
        return false;    // key not found
    }

    // Position of value start: after key + ":"
    int valStart = keyPos + strlen(key);

    // Skip spaces, colons, quotes
    while (jsonStr[valStart] == ' ' || jsonStr[valStart] == ':' || jsonStr[valStart] == '"' || jsonStr[valStart] == '\'')
    {
        valStart++;
    }

    // Find end of value: quote, comma, or closing brace
    int valEnd = valStart;
    while (jsonStr[valEnd] != '"' && jsonStr[valEnd] != '\'' && jsonStr[valEnd] != ',' && jsonStr[valEnd] != '}' && valEnd < strlen(jsonStr))
    {
        valEnd++;
    }

    int length = valEnd - valStart;
    if (length <= 0 || length >= maxlen)
    {
        return false;    // invalid value or output buffer too small
    }

    // Clear output buffer
    strcopy(out, maxlen, "");

    // Manual copy of substring
    for (int i = 0; i < length; i++)
    {
        out[i] = jsonStr[valStart + i];
    }

    out[length] = '\0';    // Null-terminate the output string

    return true;
}

// Detects if the client is valid
stock bool IsValidClient(client)
{
    if (client <= 0 || client > MaxClients || !IsClientConnected(client) || IsFakeClient(client))
    {
        return false;
    }
    return IsClientInGame(client);
}

// Starts the PTE database connection, you need to manually close the Database handle after usage
stock Database CreatePTEDatabaseConnection()
{
    char     error[256];
    Database database = SQL_Connect("default", true, error, sizeof(error));

    if (database == null)
    {
        PrintToServer("[PTE-INC] ERROR: Cannot connect to the database: %s", error);
        return null;
    }
    else {
        return database;
    }
}

// Remove 0 in string float numbers, example:
// "0.1000000" -> "0.1"
stock void TrimTrailingZeros(char[] str)
{
    int len = strlen(str);

    int dot = -1;
    for (int i = 0; i < len; i++)
    {
        if (str[i] == '.')
        {
            dot = i;
            break;
        }
    }

    if (dot == -1)
        return;

    int end = len - 1;
    while (end > dot && str[end] == '0')
    {
        str[end] = '\0';
        end--;
    }

    if (str[end] == '.')
    {
        str[end] = '\0';
    }
}
//
// #ENDREGION Utils
//