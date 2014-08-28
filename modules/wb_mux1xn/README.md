# 1xN Multiplexer/De-multiplexer with Wishbone Interface

---

##Contents

* Synopsis
* Register Layout
* Files

---

## Synopsis

This unit is used to (de-)multiplex signals. In the GSI implementation it is used to (de-)multiplex multiple UART units.

<pre>
       [CONFIGURATION REGISTER] <=> [Wishbone]
       '
       '
       '
       '
       |\
 IN0-->| \
       |  |-->OUT
 IN1-->| /
       |/
       '
       '
       '
       '
       |\
OUT1<--| \
       |  |<--IN
OUT2<--| /
       |/
</pre>

---

## Register Layout

### CONFIGURATION REGISTER @ 0x00
| Bit(s) | Reset      | R/W | Description                                                       | 
|:-------|-----------:|:---:|:------------------------------------------------------------------| 
| *      |        0x0 |  R  | -Reserved-                                                        | 
| *      |        0x0 | R/W | Read: Read selected signals. Write: Select signals.               | 

\* Depends on implementation

---

## Files

### HDL Files

* wb_mux1xn(_pkg).vhd -> Unit with wishbone interface
* wb_mux1xn_test_bench.vhd -> Simple test bench for the complete unit
