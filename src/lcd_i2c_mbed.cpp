#include "lcd_i2c_mbed.h"

// private function
void LCD_I2C_MBED::expander_writer(int data)
{
    _i2c->lock();

    _buffer[0] = static_cast<char>(data | _lcd_backlight);
    _i2c->write((_address << 1), _buffer, 1);

    _i2c->unlock();
}

void LCD_I2C_MBED::pulse_enable(int data)
{
    expander_writer(data | En); // ENABLE HIGH
    wait_us(1);                 // enable pulse must be > 450ns

    expander_writer(data & ~En);
    wait_us(50); // commands need > 37us to settle
}

void LCD_I2C_MBED::write_4_bits(int value)
{
    expander_writer(value);
    pulse_enable(value);
}

void LCD_I2C_MBED::send(int value, int mode)
{
    int high_nibble = value & 0xf0;
    int low_nibble = (value << 4) & 0xf0;

    write_4_bits(high_nibble | mode);
    write_4_bits(low_nibble | mode);
}

// public function
void LCD_I2C_MBED::command(int value)
{
    send(value, 0);
}

void LCD_I2C_MBED::write(int value)
{
    send(value, Rs);
}

void LCD_I2C_MBED::home()
{
    command(LCD_RETURNHOME); // set cursor position to zero
    wait_us(2000);           // this command takes a long time!
}

void LCD_I2C_MBED::set_i2c(I2C *i2c_bus)
{
    _i2c = i2c_bus;
}

bool LCD_I2C_MBED::is_connected() const
{
    int ack = _i2c->write((_address << 1), "1", 1);
    if (ack == 0)
    {
        return true;
    }
    return false;
}

void LCD_I2C_MBED::begin(int address, int col, int row)
{
    _address = address;
    _cols = col;
    _rows = row;
    _char_size = LCD_5x8DOTS;
    _lcd_backlight = LCD_BACKLIGHT;

    _display_function = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;

    if (_rows > 1)
    {
        _display_function |= LCD_2LINE;
    }

    // for some 1 line display you can select a 10 pixel high font
    if ((_char_size != 0) && (_rows == 1))
    {
        _display_function |= LCD_5x10DOTS;
    }

    // SEE PAGE 45/46 for initialization specs
    // according to datasheet, we need at least 40ms after power rises above 2.7v
    // before sending commands
    wait_us(50000); // 50 millisecond

    // now we pull both RS and R/W low to begin commands
    expander_writer(_lcd_backlight); // reset expanderand turn backlight off (Bit 8 = 1)
    ThisThread::sleep_for(1s);

    // put the LCD into 4 bit mode
    // this is according to the hitachi HD44780 datasheet
    // figure 24, pg 46

    // we start in 8 bit mode, try to set 4 bit mode
    write_4_bits(0x03 << 4);
    wait_us(4500); // wait min 4.1ms

    // second try
    write_4_bits(0x03 << 4);
    wait_us(4500); // wait min 4.1ms

    // third go!
    write_4_bits(0x03 << 4);
    wait_us(150);

    // finally, set to 4 bit interface
    write_4_bits(0x02 << 4);

    // set # lines, font size, etc.
    command(LCD_FUNCTIONSET | _display_function);

    // turn the display on with no cursor or blinking default
    _display_control = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
    display();

    // clear it off
    clear();

    // Inititalize to default text direction (for roman languages)
    _display_mode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;

    // set the entry mode
    command(LCD_ENTRYMODESET | _display_mode);

    home();
}

void LCD_I2C_MBED::backlight()
{
    _lcd_backlight = LCD_BACKLIGHT;
    expander_writer(0);
}

void LCD_I2C_MBED::no_backlight()
{
    _lcd_backlight = LCD_NOBACKLIGHT;
    expander_writer(0);
}

void LCD_I2C_MBED::cursor()
{
    _display_control |= LCD_CURSORON;
    command(LCD_DISPLAYCONTROL | _display_control);
}

void LCD_I2C_MBED::no_cursor()
{
    _display_control &= ~LCD_CURSORON;
    command(LCD_DISPLAYCONTROL | _display_control);
}

void LCD_I2C_MBED::set_cursor(int col, int row)
{
    int row_offsets[] = {0x00, 0x40, 0x14, 0x54};
    if (row > _rows)
    {
        row = _rows - 1; // we count rows starting w/0
    }
    command(LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

void LCD_I2C_MBED::display()
{
    _display_control |= LCD_DISPLAYON;
    command(LCD_DISPLAYCONTROL | _display_control);
}

void LCD_I2C_MBED::no_display()
{
    _display_control &= ~LCD_DISPLAYON;
    command(LCD_DISPLAYCONTROL | _display_control);
}

void LCD_I2C_MBED::clear()
{
    command(LCD_CLEARDISPLAY); // clear display, set cursor position to zero
    wait_us(2000);             // this command takes a long time!
}

void LCD_I2C_MBED::blink()
{
    _display_control |= LCD_BLINKON;
    command(LCD_DISPLAYCONTROL | _display_control);
}

void LCD_I2C_MBED::no_blink()
{
    _display_control &= ~LCD_BLINKON;
    command(LCD_DISPLAYCONTROL | _display_control);
}

// these commands scroll the display without the changing the RAM
void LCD_I2C_MBED::scroll_display_left(){   
    command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}

void LCD_I2C_MBED::scroll_display_right(){
    command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// this is for text that flows Left to Right
void LCD_I2C_MBED::left_to_right(){
    _display_mode |= LCD_ENTRYLEFT;
    command(LCD_ENTRYMODESET | _display_mode);
}

void LCD_I2C_MBED::right_to_left(){
    _display_mode &= ~LCD_ENTRYLEFT;
    command(LCD_ENTRYMODESET | _display_mode);
}

// this will 'right justify' text from the cursor
void LCD_I2C_MBED::auto_scroll(){
    _display_mode |= LCD_ENTRYSHIFTINCREMENT;
    command(LCD_ENTRYMODESET | _display_mode);
}

// this will 'left justify' text from the cursor
void LCD_I2C_MBED::no_autoscroll(){
    _display_mode &= ~LCD_ENTRYSHIFTINCREMENT;
    command(LCD_ENTRYMODESET | _display_mode);
}
