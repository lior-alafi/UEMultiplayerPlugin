Unreal Engine 5.4 Multiplayer Online Subsystem for steam plugin(listenAndServe server)

to know lobby path just right click on the lobby level copy path 
so if path is:
C:/Users/liora/Documents/Unreal Projects/multiplayerPlugin/Content/ThirdPerson/Maps/Lobby.umap
change it to:
/Game/ThirdPerson/Maps/Lobby


Installing the plugin (UE5.4)
1. copy Plugins\MultiplayerSession into your UE5 project
2. edit your project config/DefaultEngine.ini
```
[/Script/Engine.GameEngine]
+NetDriverDefinitions=(DefName="GameNetDriver",DriverClassName="OnlineSubsystemSteam.SteamNetDriver",DriverClassNameFallback="OnlineSubsystemUtils.IpNetDriver")
 
[OnlineSubsystem]
DefaultPlatformService=Steam
 
[OnlineSubsystemSteam]
bEnabled=true
SteamDevAppId=480
bInitServerOnClient=true
 
[/Script/OnlineSubsystemSteam.SteamNetDriver]
NetConnectionClassName="OnlineSubsystemSteam.SteamNetConnection"
```
3. edit config/DefaultGame.ini and add the max amount of players in the game (across all sessions)

```
[/Script/Engine.GameSession]
MaxPlayers=100
```
4. add plugin edit>plugins and install 
`online subsystem steam`  


Add to project plugin>Online Subsystem Steam
Add MultiplayerPlugin to Plugin folder
Make  a lobby level
In level blueprint construct WBP_Menu and link “menu setup” 
Notice the rest in multiplayer plugin readme
