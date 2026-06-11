`timescale 1ns/1ps

module tb_shift_subtract_divider;

//dump FSDB for verdi.
initial begin
    $fsdbDumpfile("shift_subtract_divider.fsdb");
    $fsdbDumpvars(0,"tb_shift_subtract_divider","+all");
end

reg clk;
initial begin clk=0; end
always #10 clk=~clk;

reg rst_n;
initial begin
    rst_n=0; 
    #50; 
    rst_n=1;
end


reg [15:0] dividend;
reg [15:0] divisor; 
wire [15:0] quotient;
wire [15:0] remainder;
wire done;

shift_subtract_divider my_divider(
    .iClk(clk),
    .iRst(rst_n),
    .iDividend(dividend), //beichushu.
    .iDivisor(divisor), //chushu.
    .oQuotient(quotient), //shang.
    .oRemainder(remainder), //yushu.
    .oDone(done)
);

//driven by step_i.
reg [7:0] step_i;
always @(posedge clk or negedge rst_n)
if(!rst_n) begin
    step_i<=0;
end
else begin
    case(step_i)
    0:
        if(done) begin step_i<=step_i+1; end
        else begin dividend=16'd32768; divisor=16'd100; end
    1:
        if(done) begin step_i<=step_i+1; end
        else begin dividend=16'd10; divisor=16'd5; end
    2:
        if(done) begin step_i<=step_i+1; end
        else begin dividend=16'd33; divisor=16'd12; end
    3:
        if(done) begin step_i<=step_i+1; end
        else begin dividend=16'd299; divisor=16'd39; end
    4:
        if(done) begin step_i<=step_i+1; end
        else begin dividend=16'd65535; divisor=16'd213; end
    5:
        $finish;
    endcase
end
endmodule