#ifndef LCD_I2C_MBED_H_
#define LCD_I2C_MBED_H_

#include <mbed.h>

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

// flags for backlight control
#define LCD_BACKLIGHT 0x08
#define LCD_NOBACKLIGHT 0x00

#define En 0B00000100  // Enable bit
#define Rw 0B00000010  // Read/Write bit
#define Rs 0B00000001  // Register select bit


class LCD_I2C_MBED {
    public:
        void set_i2c(I2C *i2c_bus);

        void begin(int address, int col, int row);

        // will use a dummy way to check if the i2c connected
        bool is_connected() const ;
        void clear();
        void home();
        
        void display();
        void no_display();
        void no_blink();
        void blink();
        void no_cursor();
        void cursor();
        void scroll_display_left();
        void scroll_display_right();
        void left_to_right();
        void right_to_left();
        void auto_scroll();
        void no_autoscroll();
        
        void setRowOffsets(int row1, int row2, int row3, int row4);
        void createChar(int, int[]);
        void set_cursor(int, int);
        void write(int);
        void command(int);

        void backlight();
        void no_backlight();

        template<size_t N>
        int print(const char (&value)[N]){
            int incr = 0; 
            Span<const char, N> data(value);
            for(int i=0; i < data.size() - 1; i++){
                send(data[i], Rs);
                incr += 1;
            }

            return incr;
        }

    private:
        void expander_writer(int data);
        void send(int, int);
        void write_4_bits(int);
        void pulse_enable(int);


        static constexpr int address = 0x27;
        static constexpr int alternate_address = 0x30; // TODO: check the alternative address
        
        int _address = 0;
        int _rows = 0;
        int _cols = 0;
        int _char_size = 0;
        int _lcd_backlight = 0;
        int _display_function = 0;
        int _display_control = 0;
        int _display_mode = 0;

        I2C *_i2c;
        char _buffer[20];
};

#endif