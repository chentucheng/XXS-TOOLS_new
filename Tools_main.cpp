#include<iostream>
#include<conio.h>
#include<windows.h>
using namespace std;
void back_to_main(){
	system("cls");
	cout<<" ====================================== \n";
	cout<<"|                                      |\n";
	cout<<"| W E L C O M E  T O  X X S  T O O L S |\n";
	cout<<"|                                      |\n";
	cout<<"|   [1]ScreenWings     [2]TaskKill     |\n";
	cout<<"|   [3]About...        [ESC]Close      |\n";
	cout<<"|   [4]Update Information              |\n";
	cout<<"|                                      |\n";
	cout<<" ====================================== \n";
	return;
}
void killer(){
	system("cls");
	cout<<" ====================================== \n";
	cout<<"|                                      |\n";
	cout<<"|  It will close the StudentMain.exe   |\n";
	cout<<"|  Do you KNOW ? ? ?   [Y]Yes  [N]No   |\n";
	cout<<"|                                      |\n";
	cout<<" ====================================== \n";
	int kch;
	while(1){
		if(_kbhit()){
			kch=_getch();
			if(kch==121){
				Sleep(500);
				system("taskkill -f -im studentmain.exe");
				Sleep(1000);
				back_to_main();
				return;
			}
			if(kch==110){
				back_to_main();
				return;
			}
		}
	}
}
void updn(){
	system("cls");
	cout<<" ====================================== \n";
	cout<<"|   Empty.                             |\n";
	cout<<"|                           [B]Back    |\n";
	cout<<" ====================================== \n";
	int kch;
	while(1){
		if(_kbhit()){
			kch=_getch();
			if(kch==98){
				back_to_main();
				return;
			}
		}
	}
}
void about(){
	system("cls");
	cout<<" ====================================== \n";
	cout<<"|   Made by XXS                        |\n";
	cout<<"|   2026.         :)                   |\n";
	cout<<"|                           [B]Back    |\n";
	cout<<" ====================================== \n";
	int kch;
	while(1){
		if(_kbhit()){
			kch=_getch();
			if(kch==98){
				back_to_main();
				return;
			}
		}
	}
}
void admin(){
	system("cls");
	cout<<" ====================================== \n";
	cout<<"|                                      |\n";
	cout<<"|   SuperAdmin    Root                 |\n";
	cout<<"|                           [B]Back    |\n";
	cout<<"|                                      |\n";
	cout<<" ====================================== \n";
	int kch;
	while(1){
		if(_kbhit()){
			kch=_getch();
			if(kch==98){
				back_to_main();
				return;
			}
		}
	}
}
int main(){
	cout<<" ====================================== \n";
	cout<<"|                                      |\n";
	cout<<"| W E L C O M E  T O  X X S  T O O L S |\n";
	cout<<"|                                      |\n";
	cout<<"|   [1]ScreenWings     [2]TaskKill     |\n";
	cout<<"|   [3]About...        [ESC]Close      |\n";
	cout<<"|   [4]Update Information              |\n";
	cout<<"|                                      |\n";
	cout<<" ====================================== \n";
	int ch;
	while(1){
		if(_kbhit()){
			ch = _getch();
			if(ch==49){
				system("test");
				back_to_main();
			}
			if(ch==50) killer();
			if(ch==51) about();
			if(ch==52) updn();
			if(ch==53) admin();
			if(ch==27) return 0;
		}
	}
}
