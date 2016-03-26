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
volatile int incx = 1;
volatile int incy = 1;
volatile int x = 0;
volatile int y = 20;

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

void Frame() {
  line++;

  if(line == 304) {
    pulse = 1;
    state = PrePulse;
    ICR1 = PULSE_CYCLES;
    OCR1A = SHORT_PULSE_CYCLES;
  }

  else {
    if(line > 40) {
      _delay_us(12);

/*
      if(line - 40 >= y && line -40 < y + 8) {
        for(int i = 0; i < x; i++) {
          NOP;
        }

        PORTB |= (1 << 2);
        NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; 
        NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; 
      }
*/

/*
      for(int i = 0; i < 50; i++) {
        if(i % 2) {
          PORTB |= (1 << 2);
        } else {
          PORTB &= ~(1 << 2);
        }
      }
*/
      __asm__ __volatile__(
        "sbi %0, %1\n"
        "nop\n\t"
        "cbi %0, %1\n"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        :: "I" (_SFR_IO_ADDR(PORTB)), "I" (PORTB2)
        );
      
      PORTB &= ~(1 << 2);
      
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
  x += incx;
  y += incy;

  if(x > 50 || x < 1) {
    incx *= -1;
  } 
  if(y > 200 || y < 1) {
    incy *= -1;
  }
  _delay_us(64 * 200);
}


