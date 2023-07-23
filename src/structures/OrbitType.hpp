//----------------------------------------------------------

#pragma once

//----------------------------------------------------------

enum OrbitType{
    LUNAR_DISTANCE, // distance < 0.0027 AU (MPC signature: ***)
    VERY_CLOSE, // LUNAR_DISTANCE < distance < 0.01 AU (MPC signature: !!) 
    CLOSE, // VERY_CLOSE < distance < 0.05 AU (MPC signature: !)
    STANDARD, // CLOSE < distance < MAIN_BELT
    MAIN_BELT, // object is in the main belt (MPC signature: MBAsoln)
    JUPITER_TROJAN // object is a jupiter trojan (MPC signature: JTrojansoln)
};
