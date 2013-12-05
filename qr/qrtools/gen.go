package main

import (
	"fmt"
	"io/ioutil"
	"log"
	"os/exec"
	"strings"
)

func pack(code string) []byte {
	// pad up to multiple of 8
	if len(code)%8 != 0 {
		code += strings.Repeat(" ", 8-len(code)%8)
	}
	packed := make([]byte, 0, len(code)/8)
	var byt byte
	for i, c := range code {
		if c == '#' {
			byt = byt | 1
		}
		if i%8 == 7 {
			packed = append(packed, byt)
			byt = 0
		}
		byt = byt << 1
	}
	return packed
}

func main() {
	p := make([]byte, 0)
	for h := 0; h <= 23; h++ {
		hh := h % 12
		if hh == 0 {
			hh = 12
		}
		ampm := "am"
		if h >= 12 {
			ampm = "pm"
		}
		for m := 0; m < 60; m++ {
			t := fmt.Sprintf("%d:%02d%s", hh, m, ampm)
			cmd := exec.Command("qrencode", "-s", "1", "-l", "H", "-m", "0", "-t", "ASCII", "-v", "0", t)
			b, err := cmd.Output()
			if err != nil {
				log.Fatalln(string(b), err)
			}
			code := string(b)
			// lazy but effective
			code = strings.Replace(code, "\n", "", -1)
			code = strings.Replace(code, "##", "#", -1)
			code = strings.Replace(code, "  ", " ", -1)
			p = append(p, pack(code)...)
		}
	}
	err := ioutil.WriteFile("../resources/packed/all.qr", p, 0644)
	if err != nil {
		log.Fatalln(err)
	}
}
