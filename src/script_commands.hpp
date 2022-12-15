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

    bat <flag> <enemy_type> <enemy_type> <enemy_type> <enemy_type>
        If <flag> is cleared, start a battle with enemies and set <flag>

    ebat <flag> <enemy_type> <enemy_type> <enemy_type> <enemy_type>
        Same as 'bat' but remove chunk enemy immediately (avoids glitch frame)

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

    ep <enemy_sprite> <path_name>
        Set the enemy path.

    epf <flag> <enemy_sprite> <path_name>
        If <flag> is cleared, ep <enemy_sprite> <path_name>

    st <tile> <tile_img>
        Overwrite image at <tile> with <tile_img>

    stf <tile> <flag> <tile_img>
        if <flag> is cleared, st <tile> <tile_img>

    pa <id>
        Add party member <id> to party

    obj <tx> <ty>
        Set objective marker position to <tx> <ty>

Control Instructions

    end
        Terminate current chunk script

    jmp label
        Unconditionally branch to label

    bz <reg> label
        If <reg> is zero, jmp label

    bnz <reg> label
        If <reg> is nonzero, jmp label

    bfs <flag> label
        If <flag> is set, jmp label

    bfc <flag> label
        If <flag> is cleared, jmp label

    bnst <tile> label
        If player is NOT selecting <tile>, jmp label

    bnwt <tile> label
        If player is NOT walking on <tile>, jmp label

    bnwe label
        If player is NOT walking into chunk sprite, jmp label

    bnse label
        If player is NOT selecting chunk sprite, jmp label

    bni <item flag> label
        If no party member has equipped <item flag>, jmp label

Other Assembly Syntax

    Shortcut          Meaning
    ==========================================================================
    $T                the tile on which the script object is located
    $tmsg             tmsg $T
    $tdlg             tdlg $T
    $ttp              ttp $T
    $wtp              wtp $T
    $bnst             bnst $T
    $bnwt             bnwt $T
    $st               st $T
    $stf              stf $T
    label_name:       label the position of the following instruction
    !flag_name        unique flag identifier (auto-assigns to index)
    @location         <tx> <ty> - location for teleport
    {string message}  unique string identifier (auto-assigns to index)

Asking questions:

    The syntax for {string messages} allows for embedding questions and
    responses using the pipe '|' character. A newline character should follow
    the pipe character and each following response. For example:

        msg {Would you like to pick up the item?|
        Yes
        No}

    Only up to three responses are allowed. After the player selects a
    response, the registers r1, r2, and r3 will be set to zero, except for
    the index of the response chosen, which will be set to one. For example,
    in the above prompt, if the player selected "No" then the registers would
    be set as: r1 = 0, r2 = 1, r3 = 0.

Registers:

    Register   Use
    ==========================================================================
    r0         Always zero
    r1         Nonzero if the first response to a question was selected
    r2         Nonzero if the second response to a question was selected
    r3         Nonzero if the third response to a question was selected
    r4
    r5
    r6
    r7
    r8+        Number of consumable items

*/

#pragma once

enum script_command_t
{
    CMD_END,

    CMD_MSG,
    CMD_TMSG,
    CMD_DLG,
    CMD_TDLG,
    CMD_BAT,
    CMD_EBAT,
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
    CMD_STF,
    CMD_PA,
    CMD_OBJ,

    CMD_JMP,
    CMD_BZ,
    CMD_BNZ,
    CMD_BFS,
    CMD_BFC,
    CMD_BNST,
    CMD_BNWT,
    CMD_BNWE,
    CMD_BNSE,
    CMD_BNI,
};
