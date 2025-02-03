#include <wiringPi.h>

// Pin diody LED — pin 0 w bibliotece wiringPi to BCM_GPIO 17.
// wykonując inicjowanie za pomocą funkcji wiringPiSetupSys, musimy używać numeracji BCM
// wybierając inny numer pinu, używaj numeracji BCM, a także
// zaktualizuj polecenie Strony właściwości — Zdarzenia kompilacji — Zdalne zdarzenie pokompilacyjne
// używające eksportu GPIO na potrzeby konfiguracji dla funkcji wiringPiSetupSys
#define	LED	17

int main(void)
{
	wiringPiSetupSys();

	pinMode(LED, OUTPUT);

	while (true)
	{
		digitalWrite(LED, HIGH);  // Włączone
		delay(500); // ms
		digitalWrite(LED, LOW);	  // Wyłączone
		delay(500);
	}
	return 0;
}