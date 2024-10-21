#include <stdint.h>
#include <list>

class I2C {
	public:
		I2C();
		I2C(uint16_t);
		void init(uint16_t);
		
		bool isInitialize(void);
 
		int8_t read8(void);
		bool write8(uint8_t);
  
		int8_t read8(uint8_t);
		bool write8(uint8_t, uint8_t);
  
		int16_t read16(uint8_t);
		bool write16(uint8_t, uint16_t);
		
		int16_t read16Swapped(uint8_t);
		bool write16Swapped(uint8_t, uint16_t);
		
		std::list<int8_t> readList(uint8_t, uint8_t);
		
		virtual ~I2C();
		
	private:
		int handle;
		bool initialize = false;

		union i2cUData{
			uint16_t uSData;
			uint8_t uCData[2];
		};	
		union i2cData{
			int16_t uSData;
			int8_t uCData[2];
		};	

		void swapU(i2cUData &);
		void swap(i2cData &);
};