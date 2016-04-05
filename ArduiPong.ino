#define NOP __asm__ __volatile__ ("nop\n\t")

#define CYCLES_PER_US (F_CPU / 1000000)
#define PAL_TIME_SCANLINE   64 // scanline duration
#define PAL_FRAME_SYNC      4.7
#define PAL_SYNC_PULSE      32 // sync pulse duration (2 per scanline)
#define PAL_SHORT_PULSE     2 // short pulse duration
#define PAL_LONG_PULSE      30 // long pulse duration

// us to cpu cycles
#define SCANLINE_CYCLES     ((PAL_TIME_SCANLINE * CYCLES_PER_US) + 1)
#define FRAME_SYNC_CYLCES   (PAL_FRAME_SYNC * CYCLES_PER_US) 
#define PULSE_CYCLES        (PAL_SYNC_PULSE * CYCLES_PER_US)
#define SHORT_PULSE_CYCLES  (PAL_SHORT_PULSE * CYCLES_PER_US)
#define LONG_PULSE_CYCLES   (PAL_LONG_PULSE * CYCLES_PER_US)

void (*state)() = NULL;

void setup() {
  DDRB |= B00000110;

  //PORTB |=   1 << 2;

  // init timer
  TCCR1A = _BV(COM1A1) | _BV(COM1A0) | _BV(WGM11);
  TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS10);
  TIMSK1 |= _BV(TOIE1);
  ICR1 = SCANLINE_CYCLES;
  OCR1A = FRAME_SYNC_CYLCES;
  state = Frame;
  
  // this is very iportant!
  TIMSK0 = 0; // deactivate interupt for timer 0
  sei();
}

volatile int line = 0;
volatile int pulse = 0;
volatile float incx = 1;
volatile float incy = 0.2;
volatile byte x = 0;
volatile byte y = 0;
volatile byte currentLine;
volatile float _x = 0;
volatile float _y = 0;

void VSync() {
  pulse++;
  
  if(pulse == 5) {
    // change state
    state = PostPulse;
    pulse = 0;
    OCR1A = SHORT_PULSE_CYCLES;
  }
}

void PostPulse() {
  pulse++;
  
  if(pulse == 6) {
    // change to frame
    line = 0;
    state = Frame;
    ICR1 = SCANLINE_CYCLES;
    OCR1A = FRAME_SYNC_CYLCES;
  }
}

byte buffer[] = { 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0,
                  1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0,
                  1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0,
                  1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0 };

#define FIRST_LINE 30
void drawLine() {
  currentLine = line - FIRST_LINE;

  _delay_us(12);
  __asm__ __volatile__(
    "lds r16, currentLine\n"
    "lds r17, y\n"
    "cp r16, r17\n"
    "brlo endLine\n"
    "ldi r18, 4\n"
    "add r17, r18\n" // r17 = y + 4
    "cp r16, r17\n"
    "brsh endLine\n"
    
    "ldi r16, 0\n"
    "loop0:\n"
    "nop\n"
    "lds r17, x\n"
    "cp r16, r17\n"
    "breq drawBall\n"
    "inc r16\n"
    "rjmp loop0\n"        
    "drawBall:\n"
    "sbi %0, %1\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"       
    "endLine:\n" 
    "cbi %0, %1\n"
    :: "I" (_SFR_IO_ADDR(PORTB)), "I" (PORTB2)
  );
  
  PORTB &= ~(1 << 2); 
}

void Frame() {
  line++;

  if(line == 1) {
    // sync ball x with displayed value
    x = _x < 0 ? 0 : _x;
    y = _y < 0 ? 0 : _y;
  }

  if(line == 304) {
    pulse = 1;
    state = PrePulse;
    ICR1 = PULSE_CYCLES;
    OCR1A = SHORT_PULSE_CYCLES;
  }

  else {
    if(line >= FIRST_LINE && line < 255 + FIRST_LINE) {
      drawLine();      
    }
  }
}

void PrePulse() {
  if(pulse == 5) {
    // change state
    state = VSync;
    pulse = 0;
    OCR1A = LONG_PULSE_CYCLES;
  } else {
    pulse++;
  }
}

ISR(TIMER1_OVF_vect) {
  state();

}

void loop() {
  _x += incx;
  _y += incy;

  if(_x > 80 || _x < 0) {
    incx *= -1;
  } 
  if(_y > 240 || _y < 0) {
    incy *= -1;
  }
  _delay_us(64 * 256);
}


