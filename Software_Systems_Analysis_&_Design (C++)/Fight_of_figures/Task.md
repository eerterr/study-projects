# Fight of Figures

**Time limit per test:** 1 second  
**Memory limit per test:** 256 megabytes

## Design Patterns  
Incorporate at least three of the covered design patterns that you think it makes sense. At least one design pattern from each category (Creational, Structural, Behavioral).

## Task Description  
In this assignment, your task is to simulate a board game between the green and red teams. The goal of the game is to maximize the team's score by collecting coins placed on the board.

## General Rules  
- The game is played on an N×N board.  
- The game always begins with the following entities on the board:
  - Two figures: GREEN and RED.  
  - M coins, each with a specific numerical value.  
- The green team always makes the first move.

## Figures Moving and Playing Styles  
The way of moving of each figure depends on its current playing style. The playing style for each figure can be either NORMAL or ATTACKING. Each figure has NORMAL playing style at the beginning of the game. However, the playing style of each figure can change during the game according to the following rules:

- If a figure's current style is NORMAL, it can be changed to ATTACKING.  
- If a figure's current style is ATTACKING, it can be changed to NORMAL.  
- Figures that have NORMAL playing style can move to their Von Neumann neighborhood (Image 1), while the figures that have ATTACKING style can move as shown in Image 2.

## Actions  
There are several types of actions that each team can choose:

- **Change playing style of a figure:**  
  A team can change the playing style of any figure from their team.

- **Move a figure:**  
  A team can move any figure from their team. Note that the way a figure moves depends on its current playing style.
  - A figure cannot move to a position occupied by another figure of the SAME color.  
  - If a figure moves to a position occupied by a figure of the OPPOSITE color, it will kill the enemy figure.  
  - If a figure moves to a position where a coin is located, it will collect the coin and add its value to the team's score.  
  - The figure cannot move to a location outside the board.

- **Clone a figure:**  
  A team can clone a figure from their team. If the figure that is being cloned is located on cell `[X,Y]`, the clone will appear on cell `[Y,X]`.
  - Cloning of GREEN produces GREENCLONE, while cloning of RED produces REDCLONE.  
  - A clone always initially has NORMAL playing style, regardless of the playing style of the figure it was cloned from.  
  - Each figure can be cloned only once and clones cannot be cloned. Therefore, one team can have maximum two figures on the board.  
  - Cloning is impossible if X=Y. In other words, if the figure is on the diagonal.  
  - Cloning is possible only if the target position is an empty cell.  
  - Cloning is impossible if another figure of ANY color occupies the target position.  
  - Cloning is impossible if the target position contains a coin.

## Coins and Score  
- When the figure moves to a cell that contains a coin, the value of this coin is added to the team's score and the coin disappears from the board.  
- Even when the figure is killed, the value of coins it collected during the game is NOT subtracted from the team score.  
- The team score is equal to the sum of values of coins that all figures of this team collected during the game.

## Input  
The input consists of the following:

- The first line contains an integer N (5 ≤ N ≤ 50) — the dimension of the board.  
- The second line contains two integers X1 and Y1 (1 ≤ X1,Y1 ≤ N) — the initial coordinates of the GREEN figure.  
- The third line contains two integers X2 and Y2 (1 ≤ X2,Y2 ≤ N) — the initial coordinates of the RED figure.  
- The fourth line contains an integer M (1 ≤ M ≤ 200) — the number of coins on the board.  
- The following M lines each contain three integers, X, Y (1 ≤ X,Y ≤ N), and V (1 ≤ V ≤ 1000), where X and Y are coordinates of the coin and V is its value.  
- The next line contains an integer P (1 ≤ A ≤ 100) — the number of actions played during the game.  
- The following A lines each contain two parts: `Figure` and `Action`:  
  - `Figure` has one of the following values:  
    `GREEN`, `RED`, `GREENCLONE`, `REDCLONE`  
  - `Action` has one of the following values:  
    `UP`, `DOWN`, `LEFT`, `RIGHT`, `COPY`, `STYLE`

## Guarantees  
Input is guaranteed to have proper initial board state (without violations of any game rules):

- All input limits previously defined will be satisfied (e.g., 5 ≤ N ≤ 50).  
- Green and red figures will be on different fields initially.  
- There will be no coins on the fields occupied by figures.  
- There will be no more than one coin on one field.  
- Input is guaranteed to contain actions with the proper syntax and in proper order, so you don't need to check it.  
- Odd actions (1, 3, 5, …) will always be in the format:  
  `<GREEN|GREENCLONE> <UP|DOWN|LEFT|RIGHT|COPY|STYLE>`  
- Even actions (2, 4, 6, …) will always be in the format:  
  `<RED|REDCLONE> <UP|DOWN|LEFT|RIGHT|COPY|STYLE>`  

However, actions still may be invalid due to specific game rules, and you are expected to check it.

## Additionally Invalid Actions  
An action can be invalid for the following reasons:

- **Moving action:**
  - The figure is not alive.
  - The field is occupied with the other figure of the same color.
  - The field is out of the board.

- **Cloning action:**
  - The figure is not alive.
  - The figure is located at diagonal.
  - The clone cannot be cloned.
  - The target field of cloning is occupied with the other figure.
  - The target field of cloning contains a coin.

- **Changing style:**
  - The figure is not alive.

## Output  
For each action, your program should provide the following output:

- **For cloning action:**  
  `<Figure> CLONED TO <X> <Y>` if cloning was successful.  
  `INVALID ACTION` if cloning is impossible.

- **For style changing action:**  
  `<Figure> CHANGED STYLE TO <NewStyle>` if changing style was successful, where `<NewStyle>` can be either `NORMAL` or `ATTACKING`.  
  `INVALID ACTION` if changing style is impossible (for example, the figure is not alive).

- **For moving action:**  
  `<Figure> MOVED TO <X> <Y>` if a figure moved to the empty cell.  
  `<Figure> MOVED TO <X> <Y> AND KILLED <EnemyFigure>` if a figure moved to the cell with enemy figure.  
  `<Figure> MOVED TO <X> <Y> AND COLLECTED <CoinValue>` if a figure moved to the cell with a coin.  
  `INVALID ACTION` if moving is impossible (e.g., position is out of the board).

## Game End - Scenario 2  
After executing all input actions, your program should print one of the following messages:

- `TIE. SCORE <GreenScore> <RedScore>` — if teams have equal number of points.  
- `GREEN TEAM WINS. SCORE <GreenScore> <RedScore>` — if green team has more points.  
- `RED TEAM WINS. SCORE <GreenScore> <RedScore>` — if red team has more points.

Note: There should be a new line character at the end of your output.
