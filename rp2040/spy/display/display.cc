#include "display.h"

/******************************************************************************\
|* Typedefs and enums private to the code here
\******************************************************************************/
enum REG_ADDRESSES : const uint8_t
	{
	SSD1306_CONTRAST 				= 0x81,
	SSD1306_DISPLAYALL_ON_RESUME 	= 0xA4,
	SSD1306_DISPLAYALL_ON 			= 0xA5,
	SSD1306_INVERTED_OFF 			= 0xA6,
	SSD1306_INVERTED_ON 			= 0xA7,
	SSD1306_DISPLAY_OFF 			= 0xAE,
	SSD1306_DISPLAY_ON 				= 0xAF,
	SSD1306_DISPLAYOFFSET 			= 0xD3,
	SSD1306_COMPINS 				= 0xDA,
	SSD1306_VCOMDETECT 				= 0xDB,
	SSD1306_DISPLAYCLOCKDIV 		= 0xD5,
	SSD1306_PRECHARGE 				= 0xD9,
	SSD1306_MULTIPLEX 				= 0xA8,
	SSD1306_LOWCOLUMN 				= 0x00,
	SSD1306_HIGHCOLUMN 				= 0x10,
	SSD1306_STARTLINE 				= 0x40,
	SSD1306_MEMORYMODE 				= 0x20,
	SSD1306_MEMORYMODE_HORZONTAL 	= 0x00,
	SSD1306_MEMORYMODE_VERTICAL 	= 0x01,
	SSD1306_MEMORYMODE_PAGE 		= 0x10,
	SSD1306_COLUMNADDR 				= 0x21,
	SSD1306_PAGEADDR 				= 0x22,
	SSD1306_COM_REMAP_OFF 			= 0xC0,
	SSD1306_COM_REMAP_ON 			= 0xC8,
	SSD1306_CLUMN_REMAP_OFF 		= 0xA0,
	SSD1306_CLUMN_REMAP_ON 			= 0xA1,
	SSD1306_CHARGEPUMP 				= 0x8D,
	SSD1306_EXTERNALVCC 			= 0x1,
	SSD1306_SWITCHCAPVCC 			= 0x2,
    };

static uint8_t __setup[] = 
	{
	SSD1306_DISPLAY_OFF,
	SSD1306_LOWCOLUMN,
	SSD1306_HIGHCOLUMN,
	SSD1306_STARTLINE,

	SSD1306_MEMORYMODE,
	SSD1306_MEMORYMODE_HORZONTAL,

	SSD1306_CONTRAST,
	0xFF,

	SSD1306_INVERTED_OFF,

	SSD1306_MULTIPLEX,
	63,

	SSD1306_DISPLAYOFFSET,
	0x00,

	SSD1306_DISPLAYCLOCKDIV,
	0x80,

	SSD1306_PRECHARGE,
	0x22,

	SSD1306_COMPINS,
	0x12,

	SSD1306_VCOMDETECT,
	0x40,

	SSD1306_CHARGEPUMP,
	0x14,

	SSD1306_DISPLAYALL_ON_RESUME,
	SSD1306_DISPLAY_ON
	};
	
/******************************************************************************\
|* Constructor
\******************************************************************************/
Display::Display(i2c_inst *inst, uint16_t i2cAddr, Size size)
		:_i2c(inst)
		,_address(i2cAddr)
		,_size(size)
		,_fb(nullptr)
		,_width(128)
		,_height(size == Size::W128xH32 ? 32 : 64)
		,_inverted(false)
	{
	/**************************************************************************\
	|* Create the framebuffer
	\**************************************************************************/
    _fb = new uint8_t[FRAMEBUFFER_SIZE];

	/**************************************************************************\
	|* Send all the startup commands
	\**************************************************************************/
	for (uint8_t &command: __setup) 
		_cmd(command);

	/**************************************************************************\
	|* Clear the buffer and send to the display
	\**************************************************************************/
	clear();
	update();
	}


/******************************************************************************\
|* Update the framebuffer
\******************************************************************************/
void Display::update(void)
	{
	_cmd(SSD1306_PAGEADDR); 		//Set page address from min to max
	_cmd(0x00);
	_cmd(0x07);
	_cmd(SSD1306_COLUMNADDR); 		//Set column address from min to max
	_cmd(0x00);
	_cmd(127);

	/**************************************************************************\
	|* create a temporary buffer of size of buffer plus 1 byte for startline 
	|* command aka 0x40
	\**************************************************************************/
	unsigned char data[FRAMEBUFFER_SIZE + 1];
	data[0] = SSD1306_STARTLINE;
	memcpy(data + 1, _fb, FRAMEBUFFER_SIZE);

	// send data to device
	i2c_write_blocking(_i2c, _address, data, FRAMEBUFFER_SIZE + 1, false);
	}


/******************************************************************************\
|* Clear the framebuffer
\******************************************************************************/
void Display::clear(void)
	{
    memset(_fb, 0, FRAMEBUFFER_SIZE);
	}

/******************************************************************************\
|* Set the orientation of the display by remapping the cols and rows
\******************************************************************************/
void Display::flip(bool orientation)
	{
	if (orientation) 
		{
		_cmd(SSD1306_CLUMN_REMAP_OFF);
		_cmd(SSD1306_COM_REMAP_OFF);
		} 
	else 
		{
		_cmd(SSD1306_CLUMN_REMAP_ON);
		_cmd(SSD1306_COM_REMAP_ON);
		}
	}

/******************************************************************************\
|* Set a pixel on the framebuffer
\******************************************************************************/
void Display::plot(int16_t x, int16_t y, WriteMode mode)
	{
	if ((x >= 0) && (x < _width) && (y >= 0) && (y < _height)) 
		{
        // byte to be used for buffer operation
        uint8_t byte;

        if (_size == Size::W128xH32) 
        	{
            y = (y << 1) + 1;
            byte = 1 << (y & 7);
            char byte_offset = byte >> 1;
            byte = byte | byte_offset;
        	} 
        else 
            byte = 1 << (y & 7);
        

        // check the write mode and manipulate the frame buffer
        if (mode == WriteMode::ADD) 
            _byteOR(x + (y / 8) * _width, byte);
        else if (mode == WriteMode::SUBTRACT) 
            _byteAND(x + (y / 8) * _width, ~byte);
        else if (mode == WriteMode::INVERT) 
            _byteXOR(x + (y / 8) * _width, byte);
    	}
	}


	
/******************************************************************************\
|* Draw a bitmap image to the framebuffer
\******************************************************************************/
void Display::blit(uint8_t ax, uint8_t ay, uint8_t w, uint8_t h, 
                   uint8_t *image, WriteMode mode)
	{
	for (uint8_t y = 0; y < h; y++) 
        for (uint8_t x = 0; x < w / 8; x++) 
        	{
            uint8_t byte = image[y * (w / 8) + x];
            for (uint8_t z = 0; z < 8; z++) 
                if ((byte >> (7 - z)) & 1) 
                	plot(x * 8 + z + ax, y + ay, mode);
            }
        
	}

/******************************************************************************\
|* Draw a line to the framebuffer
\******************************************************************************/
void Display::line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1,
                   WriteMode mode) 
    {
    int x, y, xe, ye, i;
   
   	int dx = x1 - x0;
    int dy = y1 - y0;
    int dx0 = (dx < 0) ? -dx : dx;
    int dy0 = (dy < 0) ? -dy : dy;
    int px = 2 * dy0 - dx0;
    int py = 2 * dx0 - dy0;
    
    if (dy0 <= dx0) 
    	{
        if (dx >= 0) 
        	{
            x = x0;
            y = y0;
            xe = x1;
        	} 
        else 
        	{
            x = x1;
            y = y1;
            xe = x0;
        	}
        plot(x, y, mode);
        
        for (i = 0; x < xe; i++) 
        	{
            x = x + 1;
            if (px < 0) 
                px = px + 2 * dy0;
            else 
            	{
                if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0)) 
                    y = y + 1;
                else 
                    y = y - 1;
                
                px = px + 2 * (dy0 - dx0);
            	}
            plot(x, y, mode);
        	}
    	} 
    else 
    	{	
        if (dy >= 0) 
        	{
            x = x0;
            y = y0;
            ye = y1;
        	} 
        else 
        	{
            x = x1;
            y = y1;
            ye = y0;
        	}
        plot(x, y, mode);
        
        for (i = 0; y < ye; i++) 
        	{
            y = y + 1;
            if (py <= 0)
                py = py + 2 * dx0;
            else 
            	{
                if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0)) 
                    x = x + 1;
                else 
                    x = x - 1;
                py = py + 2 * (dx0 - dy0);
            	}
            plot(x, y, mode);
        	}
    	}
	}


/******************************************************************************\
|* Draw a rect to the framebuffer
\******************************************************************************/
void Display::rect(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1,
                   WriteMode mode) 
	{
    line(x0, y0, x1, y0, mode);
    line(x0, y1, x1, y1, mode);
    line(x0, y0, x0, y1, mode);
    line(x1, y0, x1, y1, mode);
	}

/******************************************************************************\
|* Draw a rect to the framebuffer
\******************************************************************************/
void Display::fill(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1,
                   WriteMode mode) 
	{
    for (uint8_t x = x0; x <= x1; x++) 
        for (uint8_t y = y0; y <= y1; y++) 
            plot(x, y, mode);
	}

/******************************************************************************\
|* Draw a character on the screen at a position
\******************************************************************************/
void Display::glyph(const uint8_t *font, char c, uint8_t ax, uint8_t ay,
                    WriteMode mode, Rotation rotation) 
    {
    if ( (font != nullptr) && (c >= 32) && (c <=126))
    	{
        uint8_t fontW 	= font[0];
        uint8_t fontH 	= font[1];

        uint16_t seek 	= (c - 32) * (fontW * fontH) / 8 + 2;

        uint8_t b_seek = 0;

        for (uint8_t x = 0; x < fontW; x++) 
            for (uint8_t y = 0; y < fontH; y++) 
            	{
                if (font[seek] >> b_seek & 0b00000001) 
                    switch (rotation) 
                    	{
                        case Rotation::deg0:
                            plot(x + ax, y + ay, mode);
                            break;
                        case Rotation::deg90:
                            plot(-y + ax + fontH, x + ay, mode);
                            break;
                    	}
                	
                b_seek++;
                if (b_seek == 8) 
                	{
                    b_seek = 0;
                    seek++;
                	}
            	}
    	}
	}

/******************************************************************************\
|* Draw a string on the screen at a position
\******************************************************************************/
void Display::text(const uint8_t* font, const uint8_t *text, 
				   uint8_t x, uint8_t y, 
				   WriteMode mode, Rotation rotation) 
	{
    if ( (font != nullptr) && (text != nullptr))
		{
        uint8_t fontW = font[0];

        uint16_t n 		= 0;
        uint16_t delta	= 0;
        
        while (text[n] != '\0') 
        	{
            switch (rotation) 
            	{
                case Rotation::deg0:
                    glyph(font, text[n], x + delta, y, mode, rotation);
                    break;
                
                case Rotation::deg90:
                    glyph(font, text[n], x, y + delta, mode, rotation);
                    break;
            	}
            	
			delta += fontW;
            n++;
        	}
        }
    }

/******************************************************************************\
|* Invert the display
\******************************************************************************/
void Display::invert(void)
	{
    _cmd(SSD1306_INVERTED_OFF | !_inverted);
        _inverted = !_inverted;
	}

/******************************************************************************\
|* Set the display contrast
\******************************************************************************/
void Display::setContrast(uint8_t contrast)
	{
	_cmd(SSD1306_CONTRAST);
	_cmd(contrast);
	}

/******************************************************************************\
|* Set a new framebuffer
\******************************************************************************/
void Display::setBuffer(uint8_t *newBuffer)
	{
    delete[] _fb;
    _fb = newBuffer;
	}

/******************************************************************************\
|* Enable / Disable the display
\******************************************************************************/
void Display::enable(bool yn)
	{
	if (yn)
		_cmd(SSD1306_DISPLAY_ON);
	else
		_cmd(SSD1306_DISPLAY_OFF);
	}
	
	
#pragma mark - Private methods

/******************************************************************************\
|* Send a command to the device
\******************************************************************************/
void Display::_cmd(uint8_t cmd)
	{
	// 0x00 is a byte indicating to ssd1306 that a command is being sent
	uint8_t data[2] = {0x00, cmd};
	i2c_write_blocking(_i2c, _address, data, 2, false);
	}
	
/******************************************************************************\
|* OR data within the framebuffer
\******************************************************************************/
void Display::_byteOR(int n, uint8_t byte) 
	{
    if ((n > 0) && n < (FRAMEBUFFER_SIZE))
	    _fb[n] |= byte;
	}

/******************************************************************************\
|* AND data within the framebuffer
\******************************************************************************/
void Display::_byteAND(int n, uint8_t byte) 
	{
    if ((n > 0) && n < (FRAMEBUFFER_SIZE))
	    _fb[n] &= byte;
	}

/******************************************************************************\
|* XOR data within the framebuffer
\******************************************************************************/
void Display::_byteXOR(int n, uint8_t byte) 
	{
    if ((n > 0) && n < (FRAMEBUFFER_SIZE))
	    _fb[n] ^= byte;
	}
