use libflate::lz77::{Code, DefaultLz77EncoderBuilder, Lz77Encode, Sink};

struct GbaLz77Sink<'a> {
    i: usize,
    input: &'a [u8],
    out: Vec<u8>,
    codes: Vec<Code>,
}

impl<'a> GbaLz77Sink<'a> {
    pub fn new(input: &'a [u8]) -> Self {
        let mut sink = Self {
            i: 0,
            input,
            out: vec![],
            codes: vec![],
        };
        let length = input.len();
        sink.out.push(0x10);
        sink.out.push(length as u8);
        sink.out.push((length >> 8) as u8);
        sink.out.push((length >> 16) as u8);
        sink
    }

    fn write_codes_to_out(&mut self) {
        let mut flag_byte: u8 = 0;
        for code in &self.codes[0..8] {
            match code {
                Code::Literal(_) => {
                    flag_byte <<= 1;
                }
                Code::Pointer { .. } => {
                    flag_byte = (flag_byte << 1) | 1;
                }
            }
        }
        self.out.push(flag_byte);
        for code in &self.codes[0..8] {
            match code {
                Code::Literal(byte) => self.out.push(*byte),
                Code::Pointer {
                    length,
                    backward_distance,
                } => {
                    assert!(*length >= 3);
                    assert!(*backward_distance <= 0xFFF);
                    let n = (length - 3) as u8;
                    let dist = *backward_distance - 1;
                    let disp_lsb = (dist & 0xFF) as u8;
                    let disp_msb = (dist >> 8) as u8;
                    assert!(n <= 0xF);
                    assert!(disp_msb <= 0xF);
                    self.out.push((n << 4) | disp_msb);
                    self.out.push(disp_lsb);
                }
            }
        }

        self.codes.drain(0..8);
    }

    fn pad_final_codes(&mut self) {
        if self.codes.is_empty() {
            return;
        }
        while self.codes.len() < 8 {
            self.codes.push(Code::Literal(0));
        }
        self.write_codes_to_out();
    }
}

impl<'a> Sink for GbaLz77Sink<'a> {
    fn consume(&mut self, code: Code) {
        if let Code::Pointer { length, backward_distance: 1 } = &code {
            self.codes.push(Code::Literal(self.input[self.i]));
            if *length - 1 < 3 {
                self.codes.push(Code::Literal(self.input[self.i + 1]));
                self.codes.push(Code::Literal(self.input[self.i + 2]));
            } else {
                self.codes.push(Code::Pointer {
                    length: *length - 1,
                    backward_distance: 2,
                });
            }
        } else {
            self.codes.push(code.clone());
        }

        match &code {
            Code::Literal(_) => self.i += 1,
            Code::Pointer { length, backward_distance: _ } => {
                self.i += *length as usize;
            }
        }

        while self.codes.len() >= 8 {
            self.write_codes_to_out();
        }
    }
}

pub fn encode_gba_lz77(input: &[u8]) -> Vec<u8> {
    let mut sink = GbaLz77Sink::new(input);
    let mut encoder = DefaultLz77EncoderBuilder::new()
        .max_length(0b1111 + 3)
        .window_size(0xFFF)
        .build();

    encoder.encode(input, &mut sink);
    encoder.flush(&mut sink);
    sink.pad_final_codes();
    assert!(sink.codes.is_empty());
    sink.out
}
