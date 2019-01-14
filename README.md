# Garuna War

This project includes a single threaded game server written in C++. The game is divided in 3 main components, the network, the gameworld and a communication middleware. 

It uses a customized UDP network protocol where you can add flags to handle important packets, offset latencies, recovery or updated missed data, re-send, etc...

Players will be able to login with their account, create and select games from the lobby, fight creatures, interact with items, receive scores, experience, upgrade their status, just to mention a few.

A custom Unity client written in C# was used to interact with the server. The network communication is followed by serialized network packets. The packet constitute of a header including computer user id, unique identifier and a number count, followed by a single or multiple message headers and their respective serialized data. 

A part from the C++ standard library, some boost libraries and mysql connector, the whole server is written from scratch. It manages its own memory for performance reasons. 

## Installation

It requires boots::asio and mysql connector.

## Usage

A custom Unity client written in C# was used to interact with the server. 

## Credits

Bruno Miguel
Boost Libraries
Standard Library
MySQL
Multiplater Game Programming by Joshua Glazer and Sanjar Madhav
Game Programming Patterns by Robert Nystrom

## License

MIT

## Pictures

Picture taken in the Heartland Game Contest where it was awarded 2nd place by the Jury, and 2nd place by the people's choice.
![alt tag](http://i.imgur.com/f207Plu.jpg)

Creating an account.
![alt tag](http://i.imgur.com/gnaArZ3.jpg)

Lobby.
![alt tag](http://i.imgur.com/TXu6l3i.png)

Ingame.
![alt tag](http://i.imgur.com/iMadWWE.png)
