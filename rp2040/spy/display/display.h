#ifndef __display_header__
#define __display_header__

#include <cstring>
#include <cstdint>
#include "hardware/i2c.h"

#include "properties.h"

// For 128x32 displays it's still 1024 due to how memory mapping works on ssd1306.
#define FRAMEBUFFER_SIZE 1024


// Include the 4 system fonts (different sizes)
#define  SSD1306_ASCII_FULL
#include "5x8_font.h"
#include "8x8_font.h"
#include "12x16_font.h"
#include "16x32_font.h"

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
        	SUBTRACT,					// sets pixel off
      	  	INVERT,						// inverts pixel
			};

    	enum class Rotation
    		{
	        deg0		= 0,			// deg0 - means no rotation      
       	 	deg90,						// deg90 - means 90 deg rotation
    		};
	
	/**************************************************************************\
	|* Properties
	\**************************************************************************/
	GET(uint8_t *, fb);
	
	private:
		/**********************************************************************\
		|* Private class members
		\**********************************************************************/
		i2c_inst *		_i2c;			// The I2C instance
		uint16_t		_address;		// The I2C address
		
		Size			_size;			// Display width and height
		//uint8_t *		_fb;			// Framebuffer
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
		void plot(int16_t x, int16_t y, WriteMode mode = WriteMode::ADD);

		/**********************************************************************\
		|* Draw an image into the framebuffer
		\**********************************************************************/
        void blit(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t *image,
                            WriteMode mode = WriteMode::ADD);

		/**********************************************************************\
		|* Draw a line into the framebuffer
		\**********************************************************************/
        void line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1,
                            WriteMode mode = WriteMode::ADD);
                            
		/**********************************************************************\
		|* Draw a rect into the framebuffer
		\**********************************************************************/
        void rect(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1,
                            WriteMode mode = WriteMode::ADD);
                            
		/**********************************************************************\
		|* Fill a rect into the framebuffer
		\**********************************************************************/
        void fill(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1,
                            WriteMode mode = WriteMode::ADD);
                            
		/**********************************************************************\
		|* Draw a character on the screen at a position
		\**********************************************************************/
		void glyph(const uint8_t* font, char c, uint8_t x, uint8_t y, 
				   WriteMode mode = WriteMode::ADD, 
				   Rotation rotation = Rotation::deg0);
                            
		/**********************************************************************\
		|* Draw a character on the screen at a position
		\**********************************************************************/
		void text(const uint8_t* font, const char *txt, uint8_t x, uint8_t y, 
				   WriteMode mode = WriteMode::ADD, 
				   Rotation rotation = Rotation::deg0);

		/**********************************************************************\
		|* Set the buffer - ensure it's 1K long
		\**********************************************************************/
        void setBuffer(uint8_t *buffer);

		/**********************************************************************\
		|* Set the buffer - ensure it's 1K long
		\**********************************************************************/
        void dump(void);

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
