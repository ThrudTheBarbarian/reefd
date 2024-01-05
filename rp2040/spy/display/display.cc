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
|* Set a pixel on the framebuffer
\******************************************************************************/
void Display::setPixel(int16_t x, int16_t y, WriteMode mode)
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
|* Draw a bitmap image to the display
\******************************************************************************/
void Display::draw(int16_t ax, int16_t ay, uint8_t w, uint8_t h, 
                   uint8_t *image, WriteMode mode)
	{
	// goes over every single bit in image and sets pixel data on its coordinates
	for (uint8_t y = 0; y < h; y++) 
        for (uint8_t x = 0; x < w / 8; x++) 
        	{
            uint8_t byte = image[y * (w / 8) + x];
            for (uint8_t z = 0; z < 8; z++) 
                if ((byte >> (7 - z)) & 1) 
                	setPixel(x * 8 + z + ax, y + ay, mode);
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
