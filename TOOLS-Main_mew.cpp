#include<iostream>
#include<conio.h>
#include<windows.h>
using namespace std;
void back_to_admin_main();
void back_to_main(){
	system("cls");
	system("mode con cols=40 lines=10");
	printf(" ====================================== \n");
	printf("|                                      |\n");
	printf("| W E L C O M E  T O  X X S  T O O L S |\n");
	printf("|                                      |\n");
	printf("|   [1]ScreenWings     [2]TaskKill     |\n");
	printf("|   [3]About...        [ESC]Close      |\n");
	printf("|   [4]Update Information              |\n");
	printf("|                                      |\n");
	printf(" ====================================== \n");
	return;
}
void killer(){
	system("cls");
	system("mode con cols=40 lines=7");
	printf(" ====================================== \n");
	printf("|                                      |\n");
	printf("|  It will close the StudentMain.exe   |\n");
	printf("|  Do you KNOW ? ? ?   [Y]Yes  [N]No   |\n");
	printf("|                                      |\n");
	printf(" ====================================== \n");
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
void killer2(){
	system("cls");
	system("mode con cols=40 lines=7");
	printf(" ====================================== \n");
	printf("|                                      |\n");
	printf("|  It will close the StudentMain.exe   |\n");
	printf("|  Do you KNOW ? ? ?   [Y]Yes  [N]No   |\n");
	printf("|                                      |\n");
	printf(" ====================================== \n");
	int kch;
	while(1){
		if(_kbhit()){
			kch=_getch();
			if(kch==121){
				Sleep(500);
				system("taskkill -f -im studentmain.exe");
				Sleep(1000);
				back_to_admin_main();
				return;
			}
			if(kch==110){
				back_to_admin_main();
				return;
			}
		}
	}
}
void updn(){
	system("cls");
	system("mode con cols=40 lines=5");
	printf(" ====================================== \n");
	printf("|   Empty.                             |\n");
	printf("|                           [B]Back    |\n");
	printf(" ====================================== \n");
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
void updn2(){
	system("cls");
	system("mode con cols=40 lines=5");
	printf(" ====================================== \n");
	printf("|   Empty.                             |\n");
	printf("|                           [B]Back    |\n");
	printf(" ====================================== \n");
	int kch;
	while(1){
		if(_kbhit()){
			kch=_getch();
			if(kch==98){
				back_to_admin_main();
				return;
			}
		}
	}
}
void about(){
	system("cls");
	system("mode con cols=40 lines=6");
	printf(" ====================================== \n");
	printf("|   Made by XXS                        |\n");
	printf("|   2026.         :)                   |\n");
	printf("|                           [B]Back    |\n");
	printf(" ====================================== \n");
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
void about2(){
	system("cls");
	system("mode con cols=40 lines=6");
	printf(" ====================================== \n");
	printf("|   Made by XXS                        |\n");
	printf("|   2026.         :)                   |\n");
	printf("|                           [B]Back    |\n");
	printf(" ====================================== \n");
	int kch;
	while(1){
		if(_kbhit()){
			kch=_getch();
			if(kch==98){
				back_to_admin_main();
				return;
			}
		}
	}
}
void colours(){
	system("cls");
	system("mode con cols=40 lines=9");
	printf(" ====================================== \n");
	printf("|                                      |\n");
	printf("|   Please choose your colours.        |\n");
	printf("|   [1]Blue            [2]Red          |\n");
	printf("|   [3]Green           [4]White        |\n");
	printf("|   [B]Back                            |\n");
	printf("|                                      |\n");
	printf(" ====================================== \n");
	int ch;
	while(1){
		if(_kbhit()){
			ch = _getch();
			if(ch==49) system("color 1");
			if(ch==50) system("color 4");
			if(ch==51) system("color 2");
			if(ch==52) system("color 7");
			if(ch==98){
				back_to_admin_main();
				return;
			}
		}
	}
}
void back_to_admin_main(){
	system("cls");
	system("mode con cols=40 lines=12");
	printf(" ====================================== \n");
	printf("|                                      |\n");
	printf("| W E L C O M E  T O  X X S  T O O L S |\n");
	printf("| (S U P E R  A D M I N)               |\n");
	printf("|                                      |\n");
	printf("|   [1]ScreenWings     [2]TaskKill     |\n");
	printf("|   [3]About...        [4]Colours      |\n");
	printf("|   [5]Update Information              |\n");
	printf("|   [ESC]Close                         |\n");
	printf("|                                      |\n");
	printf(" ====================================== \n");
	int ch;
	while(1){
		if(_kbhit()){
			ch = _getch();
			if(ch==49){
				system("test");
				back_to_admin_main();
			}
			if(ch==50) killer2();
			if(ch==51) about2();
			if(ch==52) colours();
			if(ch==53) updn2();
			if(ch==27) {
				back_to_main();
				return;
			}
		}
	}
}
void admin(){
	system("cls");
	system("mode con cols=40 lines=7");
	printf(" ====================================== \n");
	printf("|                                      |\n");
	printf("| Go to the SuperAdmin moudle?         |\n");
	printf("|                     [Y]Yes  [N]No    |\n");
	printf("|                                      |\n");
	printf(" ====================================== \n");
	int kch;
	while(1){
		if(_kbhit()){
			kch=_getch();
			if(kch==121){
				back_to_admin_main();
				return;
			}
			if(kch==110){
				back_to_main();
				return;
			}
		}
	}
}
int main(){
	back_to_main();
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
			//if(ch==53) admin();
			if(ch==27) return 0;
		}
	}
}
