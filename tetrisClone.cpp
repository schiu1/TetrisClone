#include "pch.h"
#include "Windows.h"
#include <iostream>
#include <thread>
#include <vector>
using namespace std;

wstring tetromino[7];
int screenWidth = 120;
int screenHeight = 30;
int fieldWidth = 12;
int fieldHeight = 18;
unsigned char *field = nullptr;

int rotate(int x, int y, int r) {
	switch (r % 4) {
	case 0: return y * 4 + x;			//0 degrees
	case 1: return 12 + y - (x * 4);	//90 degrees
	case 2: return 15 - (y * 4) - x;	//180 degrees
	case 3: return 3 - y + (x * 4);		//270 degrees
	}
	return 0;
}

bool doesPieceFit(int tet, int rotation, int posX, int posY) {
	for (int px = 0; px < 4; px++) {
		for (int py = 0; py < 4; py++) {

			//get index of piece of tetromino
			int pi = rotate(px, py, rotation);

			//get index of position in the field 
			//using its posX and posY as starting point
			int fi = (posY + py)*fieldWidth + (posX + px);

			if (posX + px >= 0 && posX + px < fieldWidth) {
				if (posY + py >= 0 && posY + px < fieldHeight) {
					if (tetromino[tet][pi] == L'X' && field[fi] != 0)
						return false;
				}
			}
		}
	}

	return true;
}

int main()
{
	tetromino[0].append(L"..X.");
	tetromino[0].append(L"..X.");
	tetromino[0].append(L"..X.");
	tetromino[0].append(L"..X.");

	tetromino[1].append(L"..X.");
	tetromino[1].append(L".XX.");
	tetromino[1].append(L".X..");
	tetromino[1].append(L"....");

	tetromino[2].append(L".X..");
	tetromino[2].append(L".XX.");
	tetromino[2].append(L"..X.");
	tetromino[2].append(L"....");

	tetromino[3].append(L"....");
	tetromino[3].append(L".XX.");
	tetromino[3].append(L".XX.");
	tetromino[3].append(L"....");

	tetromino[4].append(L"..X.");
	tetromino[4].append(L".XX.");
	tetromino[4].append(L"..X.");
	tetromino[4].append(L"....");

	tetromino[5].append(L"..X.");
	tetromino[5].append(L"..X.");
	tetromino[5].append(L".XX.");
	tetromino[5].append(L"....");

	tetromino[6].append(L".X..");
	tetromino[6].append(L".X..");
	tetromino[6].append(L".XX.");
	tetromino[6].append(L"....");

	field = new unsigned char[fieldWidth * fieldHeight];	//create field
	for (int x = 0; x < fieldWidth; x++) {
		for (int y = 0; y < fieldHeight; y++) {		//make field boundaries 9, rest 0
			field[y * fieldWidth + x] = (x == 0 || x == fieldWidth - 1 || y == fieldHeight - 1) ? 9 : 0;
		}
	}

	wchar_t *screen = new wchar_t[screenWidth * screenHeight];
	for (int i = 0; i < screenWidth * screenHeight; i++) screen[i] = L' ';
	HANDLE console = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(console);
	DWORD dwBytesWritten = 0;

	int curPiece = 0;
	int curRotation = 0;
	int curX = fieldWidth / 2;
	int curY = 0;

	bool keys[4];
	bool rotateHold = false;
	bool gameOver = false;

	int speed = 20;
	int speedCounter = 0;
	bool forceDown = false;
	int pieceCount = 0;
	int score = 0;

	vector<int> vLines;

	while (!gameOver) {
		//GAME TIMING
		this_thread::sleep_for(50ms);
		speedCounter++;
		forceDown = (speed == speedCounter);

		//USER INPUT
		for (int k = 0; k < 4; k++) {							// R   L   D  Z
			keys[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28Z"[k]))) != 0;
		}

		//GAME LOGIC
		curX += (keys[0] && doesPieceFit(curPiece, curRotation, curX + 1, curY)) ? 1 : 0;
		curX -= (keys[1] && doesPieceFit(curPiece, curRotation, curX - 1, curY)) ? 1 : 0;
		curY += (keys[2] && doesPieceFit(curPiece, curRotation, curX, curY + 1)) ? 1 : 0;
		if (keys[3]) {
			curRotation += (!rotateHold && doesPieceFit(curPiece, curRotation + 1, curX, curY)) ? 1 : 0;
			rotateHold = true;
		}
		else {
			rotateHold = false;
		}

		if (forceDown) {
			if (doesPieceFit(curPiece, curRotation, curX, curY + 1)) {
				curY++;
			}
			else {
				//lock current piece in field
				for (int px = 0; px < 4; px++) {
					for (int py = 0; py < 4; py++) {
						if (tetromino[curPiece][rotate(px, py, curRotation)] == L'X') {
							field[(curY + py)*fieldWidth + (curX + px)] = curPiece + 1;
						}
					}
				}

				pieceCount++;
				if (pieceCount % 10 == 0) {
					speed--;
				}
				//check for horizontal lines
				for (int py = 0; py < 4; py++) {
					if (curY + py < fieldHeight - 1) {
						bool isLine = true;
						for (int px = 1; px < fieldWidth - 1; px++) {
							isLine &= (field[(curY + py)*fieldWidth + px]) != 0;
						}

						if (isLine) {
							for (int px = 1; px < fieldWidth - 1; px++) {
								field[(curY + py) * fieldWidth + px] = 8;
							}

							vLines.push_back(curY + py);
						}

					}
				}

				score += 25;
				if (!vLines.empty()) {
					score += (1 << vLines.size()) * 100;
				}

				//choose next piece
				curPiece = rand() % 7;
				curRotation = 0;
				curX = fieldWidth / 2;
				curY = 0;

				//if piece doesn't fit, game over
				gameOver = !doesPieceFit(curPiece, curRotation, curX, curY);
			}
			speedCounter = 0;
		}
		//RENDER OUTPUT


		//Draw field on screen
		for (int x = 0; x < fieldWidth; x++) {
			for (int y = 0; y < fieldHeight; y++) {
				screen[(y + 2) * screenWidth + (x + 2)] = L" ABCDEFG=#"[field[y * fieldWidth + x]];
			}
		}

		//Draw current piece
		for (int px = 0; px < 4; px++) {
			for (int py = 0; py < 4; py++) {
				if (tetromino[curPiece][rotate(px, py, curRotation)] == L'X') {
					screen[(curY + py + 2)*screenWidth + (curX + px + 2)] = curPiece + 65;
				}
			}
		}

		//Draw score
		swprintf_s(&screen[2 * screenWidth + fieldHeight + 6], 16, L"SCORE: %8d", score);

		if (!vLines.empty()) {
			WriteConsoleOutputCharacter(console, screen, screenWidth * screenHeight, { 0,0 }, &dwBytesWritten);
			this_thread::sleep_for(400ms);

			for (auto &v : vLines) {
				for (int px = 1; px < fieldWidth - 1; px++) {
					for (int py = v; py > 0; py--) {
						field[py * fieldWidth + px] = field[(py - 1) * fieldWidth + px];
					}
					field[px] = 0;
				}
			}

			vLines.clear();
		}

		// Display Frame
		WriteConsoleOutputCharacter(console, screen, screenWidth * screenHeight, { 0,0 }, &dwBytesWritten);
	}

	//On Game Over
	CloseHandle(console);
	cout << "Game Over! Score: " << score << endl;
	system("pause");

	return 0;
}
