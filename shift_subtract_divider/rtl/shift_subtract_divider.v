module shift_subtract_divider(
    input wire iClk,
    input wire iRst,
    input wire [15:0] iDividend, //beichushu.
    input wire [15:0] iDivisor, //chushu.
    output wire [15:0] oQuotient, //shang.
    output wire [15:0] oRemainder, //yushu.
    output reg oDone
);

reg [15:0] quotient_reg;
reg [15:0] remainder_reg;
assign oQuotient=quotient_reg;
assign oRemainder=remainder_reg;

//extend the temporary register to 2 times of dividend.
reg [31:0] shift_reg;
reg [15:0] cnt_iteration; 
//driven by step_i.
reg [7:0] step_i;
always @(posedge iClk or negedge iRst)
if(!iRst) begin
    step_i<=0;
    oDone<=0; 
end
else begin
    case(step_i)
    0: //initial.
        begin
            shift_reg<={16'd0,iDividend}; 
            cnt_iteration<=16-1; 
            step_i<=step_i+1;
        end
    1: //left shift 1 bit.
        begin shift_reg<=shift_reg<<1; step_i<=step_i+1; end
    2: //do compare.
        begin
            if(shift_reg[31:16]<iDivisor) begin shift_reg[0]<=0; end
            else if(shift_reg[31:16]>=iDivisor) begin 
                shift_reg[31:16]<=shift_reg[31:16]-iDivisor;
                shift_reg[0]<=1;
            end
            step_i<=step_i+1;
        end
    3:
        if(cnt_iteration==0) begin step_i<=step_i+1; end
        else begin cnt_iteration<=cnt_iteration-1; step_i<=step_i-2; end
    4:
        begin 
            quotient_reg<=shift_reg[15:0];
            remainder_reg<=shift_reg[31:16];
            step_i<=step_i+1;
        end
    5:
        begin oDone<=1; step_i<=step_i+1; end
    6:
        begin oDone<=0; step_i<=0; end
    endcase
end
endmodule