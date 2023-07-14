# BadChess
All credit for code to: Bluefever software
https://www.youtube.com/watch?v=bGAfaepBco4&list=PLZ1QII7yudbc-Ky058TEaOstZHVbT-2hg&index=1&t=3s&ab_channel=BluefeverSoftware

Current implementations:
Working Uci communication protocol, Xboard/winboard not functional
Eval function: Doubled pawns, isolated pawns, passed pawns, Piece Squares table, static material evaluation, bishop pair
Transposition/Hash table

Search:
AlphaBeta/Negamax, Quiescence Search, Killer moves, heuristic moves, Principal variation
Move ordering: MvvLva, killermoves, heuristic moves, captures, promotions
Opening book implemented with polygot
Basic movegen with 120 -> 64 based conversion
