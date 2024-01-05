#ifndef __display_header__
#define __display_header__

#include <cstring>
#include <cstdint>
#include "hardware/i2c.h"

///
/// For 128x32 displays it's still 1024 due to how memory mapping works on ssd1306.
#define FRAMEBUFFER_SIZE 1024

class Display
	{
	/**************************************************************************\
	|* Typedefs and enums
	\**************************************************************************/
	public:
	
	    enum class Size : const uint8_t 
	    	{
        	W128xH64,			   		// Display size W128xH64
			W128xH32					// Display size W128xH32
    		};

    	enum class WriteMode : const uint8_t 
    		{
       	 	ADD 		= 0,			// sets pixel on
        	SUBTRACT 	= 1,			// sets pixel off
      	  	INVERT 		= 2,			// inverts pixel
			};
	
	/**************************************************************************\
	|* Properties
	\**************************************************************************/

	
	private:
		/**********************************************************************\
		|* Private class members
		\**********************************************************************/
		i2c_inst *		_i2c;			// The I2C instance
		uint16_t		_address;		// The I2C address
		
		Size			_size;			// Display width and height
		uint8_t *		_fb;			// Framebuffer
		uint8_t 		_width;			// Framebuffer width
		uint8_t			_height;		// Framebuffer height
		bool			_inverted;		// Whether inverted
		
		/**********************************************************************\
		|* Private method: send a command to the SSD1306
		\**********************************************************************/
		void _cmd(uint8_t cmd);
	
		/**********************************************************************\
		|* Private method: OR data within the framebuffer
		\**********************************************************************/
    	void _byteOR(int n, uint8_t byte);
	
		/**********************************************************************\
		|* Private method: AND data within the framebuffer
		\**********************************************************************/
    	void _byteAND(int n, uint8_t byte);
	
		/**********************************************************************\
		|* Private method: XOR data within the framebuffer
		\**********************************************************************/
    	void _byteXOR(int n, uint8_t byte);
	
	public:
		/**********************************************************************\
		|* Constructor
		\**********************************************************************/
		explicit Display(i2c_inst *inst, uint16_t i2cAddr, Size size);

		/**********************************************************************\
		|* Set a pixel in the framebuffer
		\**********************************************************************/
		void setPixel(int16_t x, int16_t y, WriteMode mode = WriteMode::ADD);

		/**********************************************************************\
		|* Draw an image into the framebuffer
		\**********************************************************************/
        void draw(int16_t x, int16_t y, uint8_t w, uint8_t h, uint8_t *image,
                            WriteMode mode = WriteMode::ADD);

		/**********************************************************************\
		|* Set the buffer - ensure it's 1K long
		\**********************************************************************/
        void setBuffer(uint8_t *buffer);

		/**********************************************************************\
		|* Flip the display if required
		\**********************************************************************/
        void flip(bool orientation);

		/**********************************************************************\
		|* Clear the framebuffer
		\**********************************************************************/
        void clear(void);

		/**********************************************************************\
		|* Invert the framebuffer
		\**********************************************************************/
        void invert(void);

		/**********************************************************************\
		|* Set the contrast parameter
		\**********************************************************************/
        void setContrast(uint8_t contrast);

		/**********************************************************************\
		|* Turn the display on(1) or off (0)
		\**********************************************************************/
        void enable(bool enabled);

		/**********************************************************************\
		|* update the framebuffer ie: send to physical screen
		\**********************************************************************/
		void update(void);
	};
	
#endif // ! __display_header__
