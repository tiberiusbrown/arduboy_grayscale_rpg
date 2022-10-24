/*

SCRIPT SYNTAX
==============================================================================

Action Instructions

    These instructions interrupt game flow to perform an action that takes
    multiple frames and may require user input to complete. When the action
    does complete, script flow resumes after the original instruction with
    register values intact.

    msg {string}
        Show a message
        
    dlg <portrait> {string}
        Show a message with a portrait image
        
    tmsg <tile> {string}
        If player selects <tile>, msg {string}
    
    tdlg <tile> <portrait> {string}
        If player selects <tile>, dlg <portrait> {string}

    tp <tx> <ty>
        Teleport player to specified tile coordinates
        
    ttp <tile> <tx> <ty>
        If player selects <tile>, tp <tx> <ty>
        
    wtp <tile> <tx> <ty>
        If player walks onto tile, tp <tx> <ty>
        
ALU Instructions

    Note: register 0 is fixed at zero. Writes to this register do nothing.

    add <rdst> <rsrc>
        <rdst> = <rdst> + <rsrc>

    addi <rdst> <rsrc> <imm>
        <rdst> = <rsrc> + <imm>
        Note that this can be used to load immediate: addi <rdst> r0 <imm>

    sub <rdst> <rsrc>
        <rdst> = <rdst> + <rsrc>
        
Control Instructions

    end
        Terminate current chunk script

    jmp label
        Unconditionally branch to label
        
    brz <reg> label
        If <reg> is zero, jmp label
        
    brn <reg> label
        If <reg> is nonzero, jmp label
        
Other Assembly 

    Shortcut          Meaning
    ==========================================================================
    $T                the tile on which the script object is located
    $tmsg             tmsg $T
    $tdlg             tdlg $T
    $ttp              ttp $T
    $wtp              wtp $T
    label:            label the position of the following instruction

*/

#pragma once

enum {
    CMD_END,

    CMD_MSG,
    CMD_DLG,
    CMD_TMSG,
    CMD_TDLG,
    CMD_TP,
    CMD_TTP,
    CMD_WTP,
    
    CMD_ADD,
    CMD_ADDI,
    CMD_SUB,
    
    CMD_JMP,
    CMD_BRZ,
    CMD_BRN,
};
