# Garuna War

This project includes a single thread game server written in C++. Simply speaking, it is divided in 3 main cores, the network, the gameworld and a communication center that connects the gameworld with the network. 

it uses a costumized UDP network protocol where you can add flags to handle important packets, offset latencies, recovery or updated missed data, re-send, etc...

Players will be able to login with their account, create and select games from the lobby, fight creatures, interact with items, receive scores, experience, upgrade their status, just to mention a few.

A custom Unity client written in C# was used to interact with the server. The network communication is followed by serialized network packets. The packet constitute of a header including computer user id, unique identifier and a number count, followed by a single or multiple message headers and their respectuve serialized data. The client is responsible to read and write it back.

A part from the C++ standard library, some boost libraries and mysql connector, the whole server is written from scratch. It manages its own memory, and it is aimed to be cache friendly. 

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
