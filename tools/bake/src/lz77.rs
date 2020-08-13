use libflate::lz77::{Code, DefaultLz77EncoderBuilder, Lz77Encode, Sink};

struct GbaLz77Sink {
    out: Vec<u8>,
    codes: Vec<Code>,
}

impl GbaLz77Sink {
    pub fn new(length: usize) -> Self {
        let mut sink = Self {
            out: vec![],
            codes: vec![],
        };
        sink.out.push(0x10);
        sink.out.push(length as u8);
        sink.out.push((length >> 8) as u8);
        sink.out.push((length >> 16) as u8);
        sink
    }

    fn write_codes_to_out(&mut self) {
        assert_eq!(self.codes.len(), 8);
        let mut flag_byte: u8 = 0;
        for code in &self.codes {
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
        for code in &self.codes {
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

        self.codes.clear();
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

impl Sink for GbaLz77Sink {
    fn consume(&mut self, code: Code) {
        self.codes.push(code);
        if self.codes.len() == 8 {
            self.write_codes_to_out();
        }
    }
}

pub fn encode_gba_lz77(input: &[u8]) -> Vec<u8> {
    let mut sink = GbaLz77Sink::new(input.len());
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
