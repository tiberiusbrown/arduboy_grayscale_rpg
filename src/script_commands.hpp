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

    Teleportation instructions terminate the script processing, as they cause
    a chunk reload to occur.

    tp <tx> <ty>
        Teleport player to specified tile coordinates
        
    ttp <tile> <tx> <ty>
        If player selects <tile>, tp <tx> <ty>
        
    wtp <tile> <tx> <ty>
        If player walks onto <tile>, tp <tx> <ty>
        
ALU Instructions

    Note: register 0 is fixed at zero. Writes to this register do nothing.

    add <rdst> <rsrc>
        <rdst> = <rdst> + <rsrc>

    addi <rdst> <rsrc> <imm>
        <rdst> = <rsrc> + <imm>
        Note that this can be used to load immediate: addi <rdst> r0 <imm>

    sub <rdst> <rsrc>
        <rdst> = <rdst> + <rsrc>

Game Manipulation

    fs <flag>
        Set <flag>

    fc <flag>
        Clear <flag>

    ft <flag>
        Toggle <flag>

    ep <enemy_type> <path_name>
        Set the enemy path.

    epf <flag> <enemy_type> <path_name>
        If <flag> is cleared, ep <enemy_type> <path_name>
        
    st <tile> <tile_img>
        Overwrite image at <tile> with <tile_img>
        
Control Instructions

    end
        Terminate current chunk script

    jmp label
        Unconditionally branch to label

    brz <reg> label
        If <reg> is zero, jmp label

    brn <reg> label
        If <reg> is nonzero, jmp label

    brfs <flag> label
        If <flag> is set, jmp label

    brfc <flag> label
        If <flag> is cleared, jmp label
        
    brnt <tile> label
        If player is NOT selecting <tile>, jmp label
        
    brnw <tile> label
        If player is NOT walking on <tile>, jmp label

Other Assembly Syntax

    Shortcut          Meaning
    ==========================================================================
    $T                the tile on which the script object is located
    $tmsg             tmsg $T
    $tdlg             tdlg $T
    $ttp              ttp $T
    $wtp              wtp $T
    $brnt             brnt $T
    $brnw             brnw $T
    $st               st $T
    label_name:       label the position of the following instruction
    !flag_name        unique flag identifier (auto-assigns to index)
    {string message}  unique string identifier (auto-assigns to index)

*/

#pragma once

enum script_command_t {
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
    
    CMD_FS,
    CMD_FC,
    CMD_FT,
    CMD_EP,
    CMD_EPF,
    CMD_ST,
    
    CMD_JMP,
    CMD_BRZ,
    CMD_BRN,
    CMD_BRFS,
    CMD_BRFC,
    CMD_BRNT,
    CMD_BRNW,
};
