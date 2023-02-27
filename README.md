# Il Gioco Del LOTTO

This is a client-server application written in C that simulates the game of lotto. The application can be started using the Makefile provided. The server's main function is to generate lottery numbers at pre-set time intervals, receive bets from clients, and check and communicate the winners and their winnings. Clients can place bets using the client program.

## Installation

To install the Lotto Game Application, clone the repository:

`git clone https://github.com/mgiorgi13/LottoGame.git`

Navigate to the project directory and run the Makefile:

`make`

This will compile and link the server and client programs.

## Usage

To start the server, run the following command:

`./server`

This will start the server, which will begin generating lottery numbers at pre-set intervals.

To start a client, run the following command:

`./client`

This will start the client program, which will allow you to place bets on the lottery numbers generated by the server.
