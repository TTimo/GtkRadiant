//===========================================================================
// BSPC configuration file
// Quake3
//===========================================================================

#define PRESENCE_NONE               1
#define PRESENCE_NORMAL             2
#define PRESENCE_CROUCH             4

// more bounding boxes can be added if required
// always minimize the number of bounding boxes listed here to reduce AAS file size
// for instance if players cannot crouch then it's good to remove the bbox definition for it

//bounding box when running/walking
bbox    //30x30x56
{
	presencetype PRESENCE_NORMAL
		flags           0x0000
	mins            {-15, -15, -24}
	maxs            {15, 15, 32}
}

// bounding box when crouched
bbox    //30x30x40
{
	presencetype PRESENCE_CROUCH
		flags           0x0001
	mins            {-15, -15, -24}
	maxs            {15, 15, 16}
}

// do not forget settings as they might not be defaulted correctly when this cfg is used
settings
{
	// physics settings
	phys_gravitydirection       {0, 0, -1}      // direction of gravity
	phys_friction               6               // friction
	phys_stopspeed              100             // stop speed
	phys_gravity                800             // gravity
	phys_waterfriction          1               // friction in water
	phys_watergravity           400             // gravity in water
	phys_maxvelocity            320             // maximum run speed
	phys_maxwalkvelocity        320             // maximum walk speed (set for running)
	phys_maxcrouchvelocity      100             // maximum crouch speed
	phys_maxswimvelocity        150             // maximum swim speed
	phys_walkaccelerate         100             // acceleration for walking
	phys_airaccelerate          0               // acceleration flying through the air
	phys_swimaccelerate         0               // acceleration for swimming
	phys_maxstep                18              // maximum step height
	phys_maxsteepness           0.7             // maximum floor steepness a player can walk on
	phys_maxwaterjump           19              // maximum height for an out of water jump
	phys_maxbarrier             33              // maximum barrier a player can jump onto
	phys_jumpvel                270             // jump velocity
	phys_falldelta5             40              // falling delta for 5 damage ( see PM_CrashLand in game/bg_pmove.c )
	phys_falldelta10            60              // falling delta for 5 damage ( see PM_CrashLand in game/bg_pmove.c )
	// reachability settings
	// the following are all additional travel times added
	// for certain reachabilities in 1/100th of a second
	rs_waterjump                400
	rs_teleport                 50
	rs_barrierjump              100
	rs_startcrouch              300
	rs_startgrapple             500
	rs_startwalkoffledge        70
	rs_startjump                300
	rs_rocketjump               500
	rs_bfgjump                  500
	rs_jumppad                  250
	rs_aircontrolledjumppad     300
	rs_funcbob                  300
	rs_startelevator            50
	rs_falldamage5              300             // avoid getting 5 damage
	rs_falldamage10             500             // avoid getting 10 damage
	// if != 0 then this is the maximum fall height a reachability can be created for
	rs_maxfallheight            0
	// maximum height a bot may fall down when jumping to some location
	rs_maxjumpfallheight        450
}
