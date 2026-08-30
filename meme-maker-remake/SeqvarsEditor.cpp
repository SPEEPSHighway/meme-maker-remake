#include "SeqvarsEditor.h"

static Sint32 cursor;
static Sint32 baseID;
static Sint32 maxID;
Sint32 timeOfDay_forced;

struct fieldflag {
	const char* name;
	const char* desc;
	Sint32 flagID;
	Sint32 maxVal;
};

static Bool isEventFlags;

fieldflag eventFlags[] = {
	{ "EV0001", "Sonic intro", 0x0001, 1 },
	{ "EV0002", "Sonic defeats Chaos 0", 0x0002, 1 },
	{ "EV0003", "Sonic sees Tails crash", 0x0003, 1 },
	{ "EV0006", "Sonic and Tails poolside", 0x0006, 1 },
	{ "EV0007", "Sonic faces off with the Egg Hornet", 0x0007, 1 },
	{ "EV0008", "Chaos 1 emerges", 0x0008, 1 },
	{ "EV0009", "Sonic and Tails are gassed", 0x0009, 1 },
	{ "EV000B", "Chaos 4 transformation", 0x000B, 1 },
	{ "EV000C", "Sonic and Tails part ways with Knuckles", 0x000C, 1 },
	{ "EV000D", "Tornado 1 takes off", 0x000D, 1 },
	{ "EV0011", "Sonic falling into Station Square", 0x0011, 1 },
	{ "EV0012", "Amy finds Sonic", 0x0012, 1 },
	{ "EV0013", "Amy and Sonic go to Twinkle Park", 0x0013, 1 },
	{ "EV0014", "Sonic goes looking for Amy", 0x0014, 1 },
	{ "EV0015", "Sonic sees Zero and Amy", 0x0015, 1 },
	{ "EV0016", "Zero transported to the Egg Carrier", 0x0016, 1 },
	{ "EV0017", "Sonic catches up with Tails on the Tornado 2", 0x0017, 1 },
	{ "EV001A", "Eggman takes Birdie's Emerald", 0x001A, 1 },
	{ "EV001B", "Sonic goes to put Eggman out of commission", 0x001B, 1 },
	{ "EV001C", "Sonic finds Chaos 6", 0x001C, 1 },
	{ "EV001D", "Sonic jumps from the Egg Carrier into the Mystic Ruins", 0x001D, 1 },
	{ "EV001E", "Sonic sees the temple come out of the ground", 0x001E, 1 },
	{ "EV0020", "Sonic looks at the Perfect Chaos mural", 0x0020, 1 },
	{ "EV0021", "Sonic enters the Past", 0x0021, 1 },
	{ "EV0022", "Sonic listens to Tikal in the Past", 0x0022, 1 },
	{ "EV0023", "Sonic sees Eggman heading to his base", 0x0023, 1 },
	{ "EV0024", "Egg Viper", 0x0024, 1 },
	{ "EV0026", "Sonic's outro", 0x0026, 1 },
	{ "EV0028", "Sonic vs. Knuckles", 0x0028, 1 },
	{ "EV0029", "Tornado 2 lands on the Egg Carrier", 0x0029, 1 },
	{ "EV002A", "Sonic and Tails awaken after being gassed", 0x002A, 1 },
	{ "EV002B", "Sonic meets Chaos 0", 0x002B, 1 },
	{ "EV0030", "Tails intro", 0x0030, 1 },
	{ "EV0031", "Tails is rescued by Sonic", 0x0031, 1 },
	{ "EV0032", "Tails and Sonic poolside", 0x0032, 1 },
	{ "EV0033", "Tails faces off with the Egg Hornet", 0x0033, 1 },
	{ "EV0034", "Chaos 1 emerges", 0x0034, 1 },
	{ "EV0035", "Tails and Sonic are gassed at Casinopolis", 0x0035, 1 },
	{ "EV0038", "Tails vs. Knuckles", 0x0038, 1 },
	{ "EV0039", "Chaos 4 emerges", 0x0039, 1 },
	{ "EV003A", "Tails and Sonic part ways with Knuckles", 0x003A, 1 },
	{ "EV003B", "Tails and Sonic depart on the Tornado 1", 0x003B, 1 },
	{ "EV003E", "Tails' flashback", 0x003E, 1 },
	{ "EV0040", "Tails wakes up from his dream", 0x0040, 1 },
	{ "EV0042", "Tails chases Froggy", 0x0042, 1 },
	{ "EV0044", "Tails enters the Past", 0x0044, 1 },
	{ "EV0045", "Tails talks to Tikal", 0x0045, 1 },
	{ "EV0046", "Tails returns from the Past and meets Big", 0x0046, 1 },
	{ "EV0047", "The Tornado 2 takes flight", 0x0047, 1 },
	{ "EV0048", "Tails finds Sonic in Red Mountain", 0x0048, 1 },
	{ "EV004B", "Tails faces off with Gamma", 0x004B, 1 },
	{ "EV004C", "Tails escapes from the Egg Carrier with Amy", 0x004C, 1 },
	{ "EV004D", "Eggman launches his missile attack", 0x004D, 1 },
	{ "EV004E", "Tails follows Eggman after the missile", 0x004E, 1 },
	{ "EV0050", "Tails takes on the Egg Walker", 0x0050, 1 },
	{ "EV0051", "Egg Walker defeated and Station Square saved", 0x0051, 1 },
	{ "EV0052", "Tails outro", 0x0052, 1 },
	{ "EV0053", "Error", 0x0053, 1 },
	{ "EV0054", "Landing on the Egg Carrier", 0x0054, 1 },
	{ "EV0055", "Tails and Froggy go to the Past", 0x0055, 1 },
	{ "EV0056", "Tails and Sonic awake after being gassed", 0x0056, 1 },
	{ "EV0058", "Amy Pre-intro (?)", 0x0058, 1 },
	{ "EV0060", "Amy intro", 0x0060, 1 },
	{ "EV0061", "Amy meets Birdie", 0x0061, 1 },
	{ "EV0062", "Amy meets up with Sonic", 0x0062, 1 },
	{ "EV0063", "Amy and Sonic visit Twinkle Park", 0x0063, 1 },
	{ "EV0064", "Amy is kidnapped by Zero", 0x0064, 1 },
	{ "EV0065", "Amy is released by Gamma", 0x0065, 1 },
	{ "EV0066", "Amy goes to the Past after Hot Shelter", 0x0066, 1 },
	{ "EV0067", "Amy enters the Past", 0x0067, 1 },
	{ "EV0068", "Amy meets Tikal", 0x0068, 1 },
	{ "EV0069", "Eggman takes Birdie's Emerald", 0x0069, 1 },
	{ "EV006A", "Amy and Tails escape the Egg Carrier", 0x006A, 1 },
	{ "EV006B", "Error", 0x006B, 1 },
	{ "EV006C", "Amy returns to the present", 0x006C, 1 },
	{ "EV006D", "Hunt to find Birdie's family", 0x006D, 1 },
	{ "EV006E", "Amy discovers the Final Egg Base", 0x006E, 1 },
	{ "EV006F", "Amy chased by Zero in Final Egg", 0x006F, 1 },
	{ "EV0070", "Amy and Birdie head back to the Egg Carrier", 0x0070, 1 },
	{ "EV0071", "Zero confronts Amy", 0x0071, 1 },
	{ "EV0072", "Amy outro", 0x0072, 1 },
	{ "EV0075", "Amy is kidnapped to the Mystic Ruins", 0x0075, 1 },
	{ "EV0080", "Knuckles intro", 0x0080, 1 },
	{ "EV0082", "Knuckles goes hunting for the Master Emerald", 0x0082, 1 },
	{ "EV0083", "Knuckles enters the Past", 0x0083, 1 },
	{ "EV0084", "Knuckles while in the Past", 0x0084, 1 },
	{ "EV0085", "Tikal talks to her father", 0x0085, 1 },
	{ "EV0086", "Knuckles returns from the Past", 0x0086, 1 },
	{ "EV0087", "Knuckles and Chaos 2 face off", 0x0087, 1 },
	{ "EV0088", "Eggman tricks Knuckles", 0x0088, 1 },
	{ "EV0089", "Knuckles goes after Sonic", 0x0089, 1 },
	{ "EV008A", "Knuckles vs. Sonic", 0x008A, 1 },
	{ "EV008B", "Chaos 4 emerges", 0x008B, 1 },
	{ "EV008C", "Knuckles parts ways with Sonic and Tails", 0x008C, 1 },
	{ "EV008D", "Knuckles goes to the Past (from Lost World)", 0x008D, 1 },
	{ "EV008E", "Knuckles back in the Past", 0x008E, 1 },
	{ "EV008F", "Tikal talks to Chaos", 0x008F, 1 },
	{ "EV0091", "Knuckles restores most of the Master Emerald", 0x0091, 1 },
	{ "EV0092", "Knuckles follows Gamma", 0x0092, 1 },
	{ "EV0094", "Knuckles arrives on the Egg Carrier", 0x0094, 1 },
	{ "EV0095", "Knuckles finds the last missing piece", 0x0095, 1 },
	{ "EV0096", "Knuckles travels back to the Past one last time", 0x0096, 1 },
	{ "EV0097", "The aftermath of Tikal's plight", 0x0097, 1 },
	{ "EV0098", "Knuckles returns to the present", 0x0098, 1 },
	{ "EV0099", "Knuckles fights Chaos 6", 0x0099, 1 },
	{ "EV009A", "Knuckles has collected the final shards", 0x009A, 1 },
	{ "EV009B", "Knuckles defeats Chaos 6", 0x009B, 1 },
	{ "EV009C", "Error", 0x009C, 1 },
	{ "EV009D", "Knuckles restores the Master Emerald", 0x009D, 1 },
	{ "EV009F", "Knuckles outro", 0x009F, 1 },
	{ "EV00A0", "Knuckles follows Eggman in Station Square hotel", 0x00A0, 1 },
	{ "EV00B0", "Gamma intro", 0x00B0, 1 },
	{ "EV00B1", "Gamma enters Final Egg", 0x00B1, 1 },
	{ "EV00B2", "Gamma exits Final Egg", 0x00B2, 1 },
	{ "EV00B3", "Useless machine", 0x00B3, 1 },
	{ "EV00B4", "Gamma's fight with Beta", 0x00B4, 1 },
	{ "EV00B5", "Gamma defeats Beta", 0x00B5, 1 },
	{ "EV00B7", "The hunt for Froggy begins", 0x00B7, 1 },
	{ "EV00B8", "Gamma goes to the Past", 0x00B8, 1 },
	{ "EV00B9", "Gamma in the Past", 0x00B9, 1 },
	{ "EV00BA", "Gamma and Tikal meet", 0x00BA, 1 },
	{ "EV00BB", "Gamma returns to the Egg Carrier", 0x00BB, 1 },
	{ "EV00BC", "Gamma goes to the wrong room", 0x00BC, 1 },
	{ "EV00BD", "Gamma witnesses Beta being rebuilt", 0x00BD, 1 },
	{ "EV00BE", "Gamma leaves Beta's room", 0x00BE, 1 },
	{ "EV00BF", "Gamma meets and releases Amy", 0x00BF, 1 },
	{ "EV00C0", "Gamma heading to the rear of the ship", 0x00C0, 1 },
	{ "EV00C1", "Gamma emerges to fight Sonic", 0x00C1, 1 },
	{ "EV00C2", "Gamma after the battle with Sonic", 0x00C2, 1 },
	{ "EV00C3", "Gamma's objectives changed", 0x00C3, 1 },
	{ "EV00C5", "Gamma remembers his brothers", 0x00C5, 1 },
	{ "EV00C7", "Gamma outro", 0x00C7, 1 },
	{ "EV00D0", "Big intro", 0x00D0, 1 },
	{ "EV00D1", "Big goes searching for Froggy", 0x00D1, 1 },
	{ "EV00D2", "Froggy heads into the sewers", 0x00D2, 1 },
	{ "EV00D3", "Big finds Froggy with Tails", 0x00D3, 1 },
	{ "EV00D4", "Big loses Froggy to Gamma", 0x00D4, 1 },
	{ "EV00D8", "Big enters Hot Shelter", 0x00D8, 1 },
	{ "EV00D9", "Big spots Froggy inside the tanks", 0x00D9, 1 },
	{ "EV00DA", "Big saves Froggy in Hot Shelter", 0x00DA, 1 },
	{ "EV00DB", "Big heads into the Past", 0x00DB, 1 },
	{ "EV00DC", "Tikal talks to Big", 0x00DC, 1 },
	{ "EV00DD", "Big returns to the Egg Carrier", 0x00DD, 1 },
	{ "EV00DE", "Chaos 6 takes Froggy", 0x00DE, 1 },
	{ "EV00DF", "Big thanks Sonic for saving Froggy", 0x00DF, 1 },
	{ "EV00E0", "Big finds the Tornado 2 and leaves", 0x00E0, 1 },
	{ "EV00E1", "Error", 0x00E1, 1 },
	{ "EV00E2", "Big outro", 0x00E2, 1 },
	{ "EV00E3", "Big sees Froggy heading to the beach", 0x00E3, 1 },
	{ "EV00F0", "Tornado 2 Flash scene", 0x00F0, 1 },
	{ "EV00F2", "Eggman heading to the Mystic Ruins base", 0x00F2, 1 },
	{ "EV00F3", "Knuckles at the Master Emerald", 0x00F3, 1 },
	{ "EV00F4", "Tails tells Sonic about Angel Island falling", 0x00F4, 1 },
	{ "EV00F5", "Sonic and Tails find Eggman and Knuckles", 0x00F5, 1 },
	{ "EV00F6", "Sonic travels to the Past", 0x00F6, 1 },
	{ "EV00F7", "Tikal pleads with her father", 0x00F7, 1 },
	{ "EV00F8", "Tikal seals Chaos", 0x00F8, 1 },
	{ "EV00F9", "Sonic returns to the present", 0x00F9, 1 },
	{ "EV00FA", "Sonic and Tails find the Tornado 2", 0x00FA, 1 },
	{ "EV00FB", "Sonic checks on Tikal in the past", 0x00FB, 1 },
	{ "EV00FD", "Perfect Chaos reveals himself", 0x00FD, 1 },
	{ "EV00FE", "Last Story outro", 0x00FE, 1 },
	{ "EV00FF", "Everyone brings Sonic the emeralds", 0x00FF, 1 },
	{ "EV0100", "Sonic and Tails after landing on the Egg Carrier", 0x0100, 1 },
	{ "EV0101", "Don't get too many ideas you fools!", 0x0101, 1 },
	{ "EV0102", "The Egg Carrier has changed chape", 0x0102, 1 },
	{ "EV0103", "Sonic at the Sky Deck entrance", 0x0103, 1 },
	{ "EV0104", "Sonic right after Sky Deck (Is that it?)", 0x0104, 1 },
	{ "EV0106", "Sonic heading to transform the Egg Carrier", 0x0106, 1 },
	{ "EV0107", "Emergency alert cancelled", 0x0107, 1 },
	{ "EV0110", "Tails and Sonic after landing on the Egg Carrier", 0x0110, 1 },
	{ "EV0111", "Don't get too many ideas", 0x0111, 1 },
	{ "EV0112", "The Egg Carrier has changed shape", 0x0112, 1 },
	{ "EV0113", "Tails at the Sky Deck entrance", 0x0113, 1 },
	{ "EV0114", "Tails right after Sky Deck", 0x0114, 1 },
	{ "EV0120", "Egg Carrier transforms 1", 0x0120, 1 },
	{ "EV0121", "Egg Carrier transforms 2", 0x0121, 1 },
	{ "EV0122", "Knuckles sensing the emeralds on the Egg Carrier", 0x0122, 1 },
	{ "EV0130", "Introduction to Hedgehog Hammer", 0x0130, 1 },
	{ "EV0131", "Winning at Hedgehog Hammer", 0x0131, 1 },
	{ "EV0140", "Gamma is told to find the Jet Booster", 0x0140, 1 },
	{ "EV0141", "Gamma heads to Hot Shelter", 0x0141, 1 },
	{ "EV0142", "Gamma rescues E-105", 0x0142, 1 },
	{ "EV0150", "Emergency alert cancelled", 0x0150, 1 },
	{ "EV0160", "The Echidna tribe faces Chaos", 0x0160, 1 },
	{ "EV0165", "Sonic gets the Crystal Ring", 0x0165, 1 },
	{ "EV0166", "Sonic gets the Light Speed Shoes", 0x0166, 1 },
	{ "EV0167", "Sonic gets the Ancient Light", 0x0167, 1 },
	{ "EV0168", "Tails gets the Jet Anklet", 0x0168, 1 },
	{ "EV0169", "Tails gets the Rhythm Badge", 0x0169, 1 },
	{ "EV016A", "Knuckles gets the Fighting Gloves", 0x016A, 1 },
	{ "EV016B", "Knuckles gets the Shovel Claw", 0x016B, 1 },
	{ "EV016C", "Amy gets the Long Hammer", 0x016C, 1 },
	{ "EV016D", "Amy gets the Warrior Feather", 0x016D, 1 },
	{ "EV016E", "Gamma gets the Laser Blaster", 0x016E, 1 },
	{ "EV016F", "Gamma gets the Jet Booster", 0x016F, 1 },
	{ "EV0170", "Big gets the Power Rod", 0x0170, 1 },
	{ "EV0171", "Big gets the Life Belt", 0x0171, 1 },
	{ "EV0176", "Ice Stone appears (Sonic)", 0x0176, 1 },
	{ "EV0177", "Ice Stone appears (Tails)", 0x0177, 1 },
	{ "EV0178", "Ice Stone appears (Big)", 0x0178, 1 },
	{ "EV0179", "Employee Card appears", 0x0179, 1 },
	{ "EV017A", "Passage to Angel Island opens (Sonic)", 0x017A, 1 },
	{ "EV017B", "Passage to Angel Island opens (Tails)", 0x017B, 1 },
	{ "EV017C", "Passage to Angel Island opens (Gamma)", 0x017C, 1 },
	{ "EV0180", "Sonic sees the Egg Carrier in Red Mountain", 0x0180, 1 },
	{ NULL }
};

fieldflag adventureFlags[] = {
	{ "FORCED TIME OF DAY", "Overrides the time of day for SS and MR.", 0x0, 2 },
	{ "SSVAR_HOTEL_KEYBLOCK", "Big: Unused hotel puzzle completion flag", 0x0, 1 },
	{ "SSVAR_CAR_DISP", "Makes Station Square cars visible (if they exist)", 0x1, 1 },
	{ "SSVAR_TWINKLE_ELEVATOR", "Twinkle Park Elevator Status (if elevator is enabled)", 0x2, 3 },
	{ "SSVAR_EGG_ON_STAND", "Shop bars (Can only toggle when Egg stand is empty)", 0x3, 1 },
	{ "SSVAR_OFFICEDOOR_OPEN", "Opens City Hall doors", 0x4, 1 },
	{ "SSVAR_LIGHT_CONDITION", "Unused.", 0x5, 1 },

	{ "MRVAR_TORNADE_CATAPALT1", "Opens the runway at Tails' Workshop.", 0x21, 1 },
	{ "MRVAR_TORNADE_CATAPALT2", "Opens the runway door at Tails' Workshop.", 0x22, 1 },
	{ "MRVAR_TORNADE_HATCH", "Controls Tornado 2 Waterfall Entrance (0->3->2->1->0)", 0x23, 3 },
	{ "MRVAR_TORNADE2_HATCH", "Unused.", 0x24, 3  },
	{ "MRVAR_MASTEREMERALD_STATE", "Controls how complete the Master Emerald is (0->3)", 0x25, 3 },

	{ "FLAG_EC_JAIL1", "Middle EC Prison Cell", 0x21, 1 },
	{ "FLAG_EC_JAIL2", "Right EC Prison Cell", 0x22, 1 },
	{ "FLAG_EC_JAIL3", "Left EC Prison Cell", 0x23, 1 },
	{ "FLAG_EC_POOL_CTRL", "Unused. See FLAG_KNUCKLES_EC_PALMSWITCH for EC Pool.", 0x24, 3 },
	{ "FLAG_EC_ROTARYFLOOR_CTRL", "Activates the rotating floor in the main Egg Carrier interior.", 0x25, 3 },
	{ "FLAG_EC_BRIDGE_CTRL", "Closes the arched bridges in the main Egg Carrier interior.", 0x26, 1 },
	{ "FLAG_EC_MONORAIL_CTRL", "Calls the monorail if it's enabled.", 0x27, 1 },
	{ "FLAG_EC_EGGMANBUTTON", "Opens the door to the Chao Garden teleporter.", 0x28, 3 },
	{ "FLAG_EC_ELEVATOR_TOP", "Opens the doors to Eggman's bedroom and slots room.", 0x29, 1 },
	{ "FLAG_EC_ELEVATOR_BTM", "Opens the exit to Eggman's quarters.", 0x2A, 1 },
	{ "FLAG_EC_GATE_A_CTRL", "Egg Carrier Exterior Lift Gate Status.", 0x2B, 5 },
	{ "FLAG_EC_EGGLIFT_CTRL", "Egg Carrier Exterior Lift Status.", 0x2C, 5 },


	{ "SSVAR_EGG_ON", "Possibly an old version of SSVAR_EGG_ON_STAND.", 0x40, 1 },
	{ "FLAG_SONIC_PLAYABLE", "Character Select: Unlocks Sonic.", 0x41, 1 },
	{ "FLAG_MILES_PLAYABLE", "Character Select: Unlocks Tails.", 0x42, 1 },
	{ "FLAG_KNUCKLES_PLAYABLE", "Character Select: Unlocks Knuckles.", 0x43, 1 },
	{ "FLAG_AMY_PLAYABLE", "Character Select: Unlocks Amy.", 0x44, 1 },
	{ "FLAG_E102_PLAYABLE", "Character Select: Unlocks E102.", 0x45, 1 },
	{ "FLAG_BIG_PLAYABLE", "Character Select: Unlocks Big.", 0x46, 1 },
	{ "FLAG_SUPERSONIC_PLAYABLE", "Character Select: Unlocks Super Sonic", 0x47, 1 },
	{ "FLAG_SONIC_COMPLETE", "Sonic's Story Complete Flag", 0x48, 1 },
	{ "FLAG_MILES_COMPLETE", "Tails' Story Complete Flag", 0x49, 1 },
	{ "FLAG_KNUCKLES_COMPLETE", "Knuckles' Story Complete Flag", 0x4A, 1 },
	{ "FLAG_AMY_COMPLETE", "Amy's Story Complete Flag", 0x4B, 1 },
	{ "FLAG_E102_COMPLETE", "E102's Story Complete Flag", 0x4C, 1 },
	{ "FLAG_BIG_COMPLETE", "Big's Story Complete Flag", 0x4D, 1 },
	{ "FLAG_SUPERSONIC_COMPLETE", "Super Sonic's Story Complete Flag", 0x4E, 1 },
	{ "FLAG_GET_GOLDEGG", "Removes Gold Chao Egg from Station Square.", 0x4F, 1 },
	{ "FLAG_GET_SILVEREGG", "Removes Silver Chao Egg from Mystic Ruins.", 0x50, 1 },
	{ "FLAG_GET_BLACKEGG", "Removes Black Chao Egg from Egg Carrier.", 0x51, 1 },
	{ "FLAG_SONIC_CHAO_GARDEN", "Sonic: Enables Chao Garden warps.", 0x52, 1 },
	{ "FLAG_MILES_CHAO_GARDEN", "Tails: Enables Chao Garden warps.", 0x53, 1 },
	{ "FLAG_KNUCKLES_CHAO_GARDEN", "Knuckles: Enables Chao Garden warps.", 0x54, 1 },
	{ "FLAG_AMY_CHAO_GARDEN", "Amy: Enables Chao Garden warps.", 0x55, 1 },
	{ "FLAG_E102_CHAO_GARDEN", "E102: Enables Chao Garden warps.", 0x56, 1 },
	{ "FLAG_BIG_CHAO_GARDEN", "Big: Enables Chao Garden warps.", 0x57, 1 },
	{ "FLAG_PLAYING_SUPERSONIC", "Make SET Files/NPCs use Super Sonic's story files.", 0x58, 1 },
	{ "FLAG_METALSONIC_PLAYABLE", "Character Select: Unlocks Metal Sonic", 0x59, 1 },
	{ "FLAG_SONIC_SS_BARRICADE", "Sonic: Removes the police barricade.", 0x80, 1 },
	{ "FLAG_SONIC_SS_HOTEL_FRONT", "Sonic: Opens the hotel's front door.", 0x81, 1 },
	{ "FLAG_SONIC_SS_STATION_FRONT", "Sonic: Opens the station.", 0x82, 1 },
	{ "FLAG_SONIC_SS_ENTRANCE_SEWER", "Sonic: Removes the car over the sewer entrance.", 0x83, 1 },
	{ "FLAG_SONIC_SS_EXIT_SEWER", "Sonic: Has escaped the sewer at least once. (Only used for story sequencing)", 0x84, 1 },
	{ "FLAG_SONIC_SS_ICESTONE", "Sonic: SS Ice Stone exists. (if MR_ICESTONE is OFF)", 0x85, 1 },
	{ "FLAG_SONIC_SS_HOTEL_BACK", "Sonic: Opens the hotel's back door.", 0x86, 1 },
	{ "FLAG_SONIC_SS_ENTRANCE_CASINO", "Sonic: Opens the Casinopolis entrance.", 0x87, 1 },
	{ "FLAG_SONIC_SS_STATION_BACK", "Sonic: Opens the station's back door.", 0x88, 1 },
	{ "FLAG_SONIC_SS_TPARK_ELEVATOR", "Sonic: Enables the Twinkle Park elevator.", 0x89, 1 },
	{ "FLAG_SONIC_SS_ENTRANCE_CIRCUIT", "Sonic: Enables the Twinkle Circuit door.", 0x8A, 1 },
	{ "FLAG_SONIC_SS_CARD", "Sonic: ID card exists. (if SS_ENTRANCE_HIGHWAY is OFF)", 0x8B, 1 },
	{ "FLAG_SONIC_SS_ENTRANCE_HIGHWAY", "Sonic: Opens the shutter to Speed Highway.", 0x8C, 1 },
	{ "FLAG_SONIC_SS_LIGHTSHOOSE", "Sonic: Has Light Shoes.", 0x8D, 1 },
	{ "FLAG_SONIC_SS_CRYSTALRING", "Sonic: Has Crystal Ring.", 0x8E, 1 },
	{ "FLAG_SONIC_EC_MONORAIL", "Sonic: Enables Monorail.", 0x8F, 1 },
	{ "FLAG_SONIC_EC_EGGLIFT", "Sonic: Enables Egg Carrier spinning elevator.", 0x90, 1 },
	{ "FLAG_SONIC_EC_TRANSFORM", "Sonic: Transforms Egg Carrier.", 0x91, 1 },
	{ "FLAG_SONIC_EC_TORNADO2_LOST", "Sonic: Removes crashed Tornado 2. (if EC_SINK is OFF)", 0x92, 1 },
	{ "FLAG_SONIC_EC_SINK", "Sonic: Egg Carrier in ocean.", 0x93, 1 },
	{ "FLAG_SONIC_MR_WINDYSTONE", "Sonic: Wind Stone exists (if MR_ENTRANCE_WINDY is OFF)", 0x94, 1 },
	{ "FLAG_SONIC_MR_ENTRANCE_WINDY", "Sonic: Opens Windy Valley", 0x95, 1 },
	{ "FLAG_SONIC_MR_WESTROCK", "Sonic: Opens Angel Island cave.", 0x96, 1 },
	{ "FLAG_SONIC_MR_ICESTONE", "Sonic: Ice Stone exists (if MR_ENTRANCE_ICECAP is OFF)", 0x97, 1 },
	{ "FLAG_SONIC_MR_ENTRANCE_ICECAP", "Sonic: Opens Icecap door.", 0x98, 1 },
	{ "FLAG_SONIC_MR_ENTRANCE_MOUNTAIN", "Sonic: Opens Red Mountain gate.", 0x99, 1 },
	{ "FLAG_SONIC_MR_ISLANDDOOR", "Sonic: Opens Angel Island cave door.", 0x9A, 1 },
	{ "FLAG_SONIC_MR_TRUCK", "Sonic: Spawns Jungle minecart.", 0x9B, 1 },
	{ "FLAG_SONIC_MR_ENTRANCE_RUIN", "Sonic: Opens Lost World.", 0x9C, 1 },
	{ "FLAG_SONIC_MR_APPEAR_FINALEGG", "Sonic: Spawns Eggman's base in the jungle.", 0x9D, 1 },
	{ "FLAG_SONIC_MR_ENTRANCE_SANDBOARD", "Sonic: Opens Sand Hill.", 0x9E, 1 },
	{ "FLAG_SONIC_MR_ANCIENT_LIGHT", "Sonic: Has Ancient Light.", 0x9F, 1 },
	{ "FLAG_SONIC_MR_ENTRANCE_FINALEGG", "Sonic: Opens Final Egg.", 0xA0, 1 },
	{ "FLAG_SONIC_TRAIN", "Sonic: Enables SS <-> MR Train.", 0xA1, 1 },
	{ "FLAG_SONIC_BOAT", "Sonic: Enables SS <-> EC Boat.", 0xA2, 1 },
	{ "FLAG_SONIC_RAFT", "Sonic: Enables MR <-> EC Raft.", 0xA3, 1 },
	{ "FLAG_SONIC_MBOSS_E102", "Sonic: Cleared E102. (Boss)", 0xA4, 1 },
	{ "FLAG_SONIC_MBOSS_KNUCKLES", "Sonic: Cleared Knuckles. (Boss)", 0xA5, 1 },
	{ "FLAG_SONIC_CLEAR_BEACH", "Sonic: Cleared Emerald Coast.", 0xA6, 1 },
	{ "FLAG_SONIC_CLEAR_WINDY", "Sonic: Cleared Windy Valley.", 0xA7, 1 },
	{ "FLAG_SONIC_CLEAR_CASINO", "Sonic: Cleared Casinopolis.", 0xA8, 1 },
	{ "FLAG_SONIC_CLEAR_TWINKLEPARK", "Sonic: Cleared Twinkle Park.", 0xA9, 1 },
	{ "FLAG_SONIC_CLEAR_HIGHWAY", "Sonic: Cleared Speed Highway.", 0xAA, 1 },
	{ "FLAG_SONIC_CLEAR_MOUNTAIN", "Sonic: Cleared Red Mountain.", 0xAB, 1 },
	{ "FLAG_SONIC_CLEAR_SNOW", "Sonic: Cleared Icecap.", 0xAC, 1 },
	{ "FLAG_SONIC_CLEAR_SKYDECK", "Sonic: Cleared Sky Deck.", 0xAD, 1 },
	{ "FLAG_SONIC_CLEAR_RUIN", "Sonic: Cleared Lost World.", 0xAE, 1 },
	{ "FLAG_SONIC_CLEAR_FINALEGG", "Sonic: Cleared Final Egg.", 0xAF, 1 },
	{ "FLAG_SONIC_CLEAR_CHAOS0", "Sonic: Cleared Chaos 0.", 0xB0, 1 },
	{ "FLAG_SONIC_CLEAR_CHAOS4", "Sonic: Cleared Chaos 4.", 0xB1, 1 },
	{ "FLAG_SONIC_CLEAR_CHAOS6", "Sonic: Cleared Chaos 6.", 0xB2, 1 },
	{ "FLAG_SONIC_CLEAR_EGGMOBILE1", "Sonic: Cleared Egg Hornet.", 0xB3, 1 },
	{ "FLAG_SONIC_CLEAR_EGGMOBILE3", "Sonic: Cleared Egg Viper.", 0xB4, 1 },
	{ "FLAG_SONIC_CLEAR_SHOOTING", "Sonic: Cleared Sky Chase 1.", 0xB5, 1 },
	{ "FLAG_SONIC_CLEAR_SHOOTING2", "Sonic: Cleared Sky Chase 2.", 0xB6, 1 },
	{ "FLAG_SONIC_ARRIVE_IN_SS", "Sonic: Enables SS Chao Garden warp.", 0xB7, 1 },
	{ "FLAG_SONIC_ARRIVE_IN_EC", "Sonic: Enables EC Chao Garden warp.", 0xB8, 1 },
	{ "FLAG_SONIC_ARRIVE_IN_MR", "Sonic: Enables MR Chao Garden warp.", 0xB9, 1 },
	{ "FLAG_MILES_SS_BARRICADE", "Tails: Removes the police barricade.", 0xC0, 1 },
	{ "FLAG_MILES_SS_HOTEL_FRONT", "Tails: Opens the hotel's front door.", 0xC1, 1 },
	{ "FLAG_MILES_SS_HOTEL_BACK", "Tails: Opens the hotel's back door.", 0xC2, 1 },
	{ "FLAG_MILES_SS_HOTEL_POOL", "Tails: Unused.", 0xC3, 1 },
	{ "FLAG_MILES_SS_ICESTONE", "Tails: SS Ice Stone exists. (if MR_ICESTONE is OFF)", 0xC4, 1 },
	{ "FLAG_MILES_SS_ENTRANCE_CASINO", "Tails: Opens the Casinopolis entrance.", 0xC5, 1 },
	{ "FLAG_MILES_SS_ENTRANCE_HIGHWAY", "Tails: Breaks the shutter to Speed Highway.", 0xC6, 1 },
	{ "FLAG_MILES_SS_TPARK_ELEVATOR", "Tails: Enables the Twinkle Park elevator.", 0xC7, 1 },
	{ "FLAG_MILES_SS_ENTRANCE_CIRCUIT", "Tails: Enables the Twinkle Circuit door.", 0xC8, 1 },
	{ "FLAG_MILES_SS_CARTPASSPORT", "Tails: Cart Pass exists (if SS_ENTRANCE_CIRCUIT is OFF)", 0xC9, 1 },
	{ "FLAG_MILES_SS_STATION_FRONT", "Tails: Opens the station.", 0xCA, 1 },
	{ "FLAG_MILES_SS_STATION_BACK", "Tails: Opens the station's back door.", 0xCB, 1 },
	{ "FLAG_MILES_SS_HOTELSWITCH", "Tails: Unused.", 0xCC, 1 },
	{ "FLAG_MILES_SS_JETANKLET", "Tails: Has Jet Anklet.", 0xCD, 1 },
	{ "FLAG_MILES_EC_MONORAIL", "Tails: Enables Monorail.", 0xCE, 1 },
	{ "FLAG_MILES_EC_EGGLIFT", "Tails: Enables Egg Carrier spinning elevator.", 0xCF, 1 },
	{ "FLAG_MILES_EC_TRANSFORM", "Tails: Transforms Egg Carrier.", 0xD0, 1 },
	{ "FLAG_MILES_EC_TORNADO2_LOST", "Tails: Removes crashed Tornado 2. (if EC_SINK is OFF)", 0xD1, 1 },
	{ "FLAG_MILES_EC_SINK", "Tails: Egg Carrier in ocean.", 0xD2, 1 },
	{ "FLAG_MILES_MR_WINDYSTONE", "Tails: Wind Stone exists (if MR_ENTRANCE_WINDY is OFF)", 0xD3, 1 },
	{ "FLAG_MILES_MR_ENTRANCE_WINDY", "Tails: Opens Windy Valley.", 0xD4, 1 },
	{ "FLAG_MILES_MR_WESTROCK", "Tails: Opens Angel Island cave.", 0xD5, 1 },
	{ "FLAG_MILES_MR_ICESTONE", "Tails: Ice Stone exists (if MR_ENTRANCE_ICECAP is OFF)", 0xD6, 1 },
	{ "FLAG_MILES_MR_ENTRANCE_ICECAP", "Tails: Opens Icecap door.", 0xD7, 1 },
	{ "FLAG_MILES_MR_TRUCK", "Tails: Spawns Jungle minecart.", 0xD8, 1 },
	{ "FLAG_MILES_MR_ENTRANCE_RUIN", "Tails: Unused.", 0xD9, 1 },
	{ "FLAG_MILES_MR_ENTRANCE_SANDBOARD", "Tails: Opens Sand Hill.", 0xDA, 1 },
	{ "FLAG_MILES_MR_RHYTHMBROOCH", "Tails: Has Rhythm Badge", 0xDB, 1 },
	{ "FLAG_MILES_TRAIN", "Tails: Enables SS <-> MR Train.", 0xDC, 1 },
	{ "FLAG_MILES_BOAT", "Tails:  Enables SS <-> EC Boat.", 0xDD, 1 },
	{ "FLAG_MILES_RAFT", "Tails: Enables MR <-> EC Raft.", 0xDE, 1 },
	{ "FLAG_MILES_MBOSS_E102", "Tails: Cleared E102. (Boss)", 0xDF, 1 },
	{ "FLAG_MILES_MBOSS_KNUCKLES", "Tails: Cleared Knuckles. (Boss)", 0xE0, 1 },
	{ "FLAG_MILES_CLEAR_WINDY", "Tails: Cleared Windy Valley.", 0xE1, 1 },
	{ "FLAG_MILES_CLEAR_CASINO", "Tails: Cleared Casinopolis.", 0xE2, 1 },
	{ "FLAG_MILES_CLEAR_HIGHWAY", "Tails: Cleared Speed Highway.", 0xE3, 1 },
	{ "FLAG_MILES_CLEAR_SNOW", "Tails: Cleared Icecap.", 0xE4, 1 },
	{ "FLAG_MILES_CLEAR_SKYDECK", "Tails: Cleared Sky Deck.", 0xE5, 1 },
	{ "FLAG_MILES_CLEAR_SANDBOARD", "Tails: Cleared Sand Hill.", 0xE6, 1 },
	{ "FLAG_MILES_CLEAR_CHAOS4", "Tails: Cleared Chaos 4.", 0xE7, 1 },
	{ "FLAG_MILES_CLEAR_EGGMOBILE2", "Tails: Cleared Egg Walker.", 0xE8, 1 },
	{ "FLAG_MILES_CLEAR_EGGMOBILE1", "Tails: Cleared Egg Hornet.", 0xE9, 1 },
	{ "FLAG_MILES_CLEAR_SHOOTING", "Tails: Cleared Sky Chase 1.", 0xEA, 1 },
	{ "FLAG_MILES_CLEAR_SHOOTING2", "Tails: Cleared Sky Chase 2.", 0xEB, 1 },
	{ "FLAG_MILES_CLEAR_BEACH", "Tails: Cleared Emerald Coast. (Cutscene)", 0xEC, 1 },
	{ "FLAG_MILES_CLEAR_MOUNTAIN", "Tails: Cleared Red Mountain. (Cutscene)", 0xED, 1 },
	{ "FLAG_MILES_ARRIVE_IN_SS", "Tails: Enables SS Chao Garden warp.", 0xEE, 1 },
	{ "FLAG_MILES_ARRIVE_IN_EC", "Tails: Enables EC Chao Garden warp.", 0xEF, 1 },
	{ "FLAG_MILES_ARRIVE_IN_MR", "Tails: Enables MR Chao Garden warp.", 0xF0, 1 },
	{ "FLAG_KNUCKLES_SS_BARRICADE", "Knuckles: Removes City Hall Barricade", 0x100, 1 },
	{ "FLAG_KNUCKLES_SS_HOTEL_FRONT", "Knuckles: Opens the hotel's front door.", 0x101, 1 },
	{ "FLAG_KNUCKLES_SS_HOTEL_BACK", "Knuckles: Opens the hotel's back door.", 0x102, 1 },
	{ "FLAG_KNUCKLES_SS_ENTRANCE_CASINO", "Knuckles: Opens the Casinopolis entrance.", 0x103, 1 },
	{ "FLAG_KNUCKLES_SS_ENTRANCE_HIGHWAY", "Knuckles: Opens the City Hall doors when approached.", 0x104, 1 },
	{ "FLAG_KNUCKLES_SS_TPARK_ELEVATOR", "Knuckles: Enables the Twinkle Park elevator.", 0x105, 1 },
	{ "FLAG_KNUCKLES_SS_ENTRANCE_CIRCUIT", "Knuckles: Enables the Twinkle Circuit door.", 0x106, 1 },
	{ "FLAG_KNUCKLES_SS_CARTPASSPORT", "Knuckles: Cart Pass exists (if SS_ENTRANCE_CIRCUIT is OFF)", 0x107, 1 },
	{ "FLAG_KNUCKLES_SS_STATION_FRONT", "Knuckles: Opens the station.", 0x108, 1 },
	{ "FLAG_KNUCKLES_SS_STATION_BACK", "Knuckles: pens the station's back door.", 0x109, 1 },
	{ "FLAG_KNUCKLES_EC_MONORAIL", "Knuckles: Enables Monorail.", 0x10A, 1 },
	{ "FLAG_KNUCKLES_EC_EGGLIFT", "Knuckles: Enables Egg Carrier spinning elevator.", 0x10B, 1 },
	{ "FLAG_KNUCKLES_EC_TRANSFORM", "Knuckles: Transforms Egg Carrier.", 0x10C, 1 },
	{ "FLAG_KNUCKLES_EC_PALMSWITCH", "Drains Egg Carrier Pool.", 0x10D, 1 },
	{ "FLAG_KNUCKLES_EC_TORNADO2_LOST", "Knuckles: Removes crashed Tornado 2. (if EC_SINK is OFF)", 0x10E, 1 },
	{ "FLAG_KNUCKLES_EC_SINK", "Knuckles: Egg Carrier in ocean.", 0x10F, 1 },
	{ "FLAG_KNUCKLES_MR_ENTRANCE_MOUNTAIN", "Knuckles: Opens Red Mountain gate.", 0x110, 1 },
	{ "FLAG_KNUCKLES_MR_WESTROCK", "Knuckles: Opens Angel Island cave.", 0x111, 1 },
	{ "FLAG_KNUCKLES_MR_ISLANDDOOR", "Knuckles: Opens Angel Island cave door.", 0x112, 1 },
	{ "FLAG_KNUCKLES_MR_MONKEYDOOR", "Knuckles: Opens monkey switch cave door.", 0x113, 1 },
	{ "FLAG_KNUCKLES_MR_TRUCK", "Knuckles: Spawns Jungle minecart.", 0x114, 1 },
	{ "FLAG_KNUCKLES_MR_REDCUBE", "Knuckles: Gold statue inserted.", 0x115, 1 },
	{ "FLAG_KNUCKLES_MR_BLUECUBE", "Knuckles: Silver statue inserted.", 0x116, 1 },
	{ "FLAG_KNUCKLES_MR_ENTRANCE_RUIN", "Knuckles: Opens Lost World.", 0x117, 1 },
	{ "FLAG_KNUCKLES_MR_APPEAR_FINALEGG", "Knuckles: Spawns Eggman's (locked) base in the jungle.", 0x118, 1 },
	{ "FLAG_KNUCKLES_MR_SHOVELCLAW", "Knuckles: Has Shovel Claw.", 0x119, 1 },
	{ "FLAG_KNUCKLES_MR_FIGHTINGGROVE", "Knuckles: Has Fighting Gloves.", 0x11A, 1 },
	{ "FLAG_KNUCKLES_TRAIN", "Knuckles: Enables SS <-> MR Train.", 0x11B, 1 },
	{ "FLAG_KNUCKLES_BOAT", "Knuckles: Enables SS <-> EC Boat.", 0x11C, 1 },
	{ "FLAG_KNUCKLES_RAFT", "Knuckles: Enables MR <-> EC Raft.", 0x11D, 1 },
	{ "FLAG_KNUCKLES_MBOSS_SONIC", "Knuckles: Cleared Sonic. (Boss)", 0x11E, 1 },
	{ "FLAG_KNUCKLES_CLEAR_CASINO", "Knuckles: Cleared Casinopolis.", 0x11F, 1 },
	{ "FLAG_KNUCKLES_CLEAR_HIGHWAY", "Knuckles: Cleared Speed Highway.", 0x120, 1 },
	{ "FLAG_KNUCKLES_CLEAR_MOUNTAIN", "Knuckles: Cleared Red Mountain.", 0x121, 1 },
	{ "FLAG_KNUCKLES_CLEAR_RUIN", "Knuckles: Cleared Lost World.", 0x122, 1 },
	{ "FLAG_KNUCKLES_CLEAR_CHAOS2", "Knuckles: Cleared Chaso 2.", 0x123, 1 },
	{ "FLAG_KNUCKLES_CLEAR_CHAOS6", "Knuckles: Cleared Chaos 6.", 0x124, 1 },
	{ "FLAG_KNUCKLES_CLEAR_CHAOS4", "Knuckles: Cleared Chaos 4.", 0x125, 1 },
	{ "FLAG_KNUCKLES_CLEAR_SKYDECK", "Knuckles: Cleared Sky Deck.", 0x126, 1 },
	{ "FLAG_KNUCKLES_ARRIVE_IN_SS", "Knuckles: Enables SS Chao Garden warp.", 0x127, 1 },
	{ "FLAG_KNUCKLES_ARRIVE_IN_EC", "Knuckles: Enables EC Chao Garden warp.", 0x128, 1 },
	{ "FLAG_KNUCKLES_ARRIVE_IN_MR", "Knuckles: Enables MR Chao Garden warp.", 0x129, 1 },
	{ "FLAG_KNUCKLES_MR_MONKEYDOOR_ENTER", "Knuckles: Has entered monkey switch cave.", 0x12A, 1 },
	{ "FLAG_KNUCKLES_MR_MONKEYCAGEA_BOMB", "Knuckles: Has cleared monkey switch cave.", 0x12B, 1 },
	{ "FLAG_KNUCKLES_SS_ENTRANCE_CHAOS2", "Knuckles: Enables Chaos 2 elevator (if CLEAR_CHAOS2 is OFF).", 0x12C, 1 },
	{ "FLAG_AMY_SS_HOTEL_FRONT", "Amy: Opens the hotel's front door.", 0x140, 1 },
	{ "FLAG_AMY_SS_HOTEL_BACK", "Amy: Opens the hotel's back door.", 0x141, 1 },
	{ "FLAG_AMY_SS_HOTEL_POOL", "Amy: Unused.", 0x142, 1 },
	{ "FLAG_AMY_SS_ENTRANCE_CASINO", "Amy: Unused. (On Dreamcast, it opened Casinopolis)", 0x143, 1 },
	{ "FLAG_AMY_SS_TPARK_ELEVATOR", "Amy: Enables the Twinkle Park elevator.", 0x144, 1 },
	{ "FLAG_AMY_SS_ENTRANCE_CIRCUIT", "Amy: Enables the Twinkle Circuit door.", 0x145, 1 },
	{ "FLAG_AMY_SS_STATION_FRONT", "Amy: Opens the station.", 0x146, 1 },
	{ "FLAG_AMY_SS_STATION_BACK", "Amy: Opens the station's back door.", 0x147, 1 },
	{ "FLAG_AMY_EC_MONORAIL", "Amy: Enables Monorail.", 0x148, 1 },
	{ "FLAG_AMY_EC_EGGLIFT", "Amy: Enables Egg Carrier spinning elevator.", 0x149, 1 },
	{ "FLAG_AMY_EC_TRANSFORM", "Amy: Transforms Egg Carrier.", 0x14A, 1 },
	{ "FLAG_AMY_EC_MOGURATATAKI", "Amy: Cleared Hedgehog Hammer", 0x14B, 1 },
	{ "FLAG_AMY_EC_TORNADO2_LOST", "Amy: Removes crashed Tornado 2. (if EC_SINK is OFF)", 0x14C, 1 },
	{ "FLAG_AMY_EC_SINK", "Amy: Egg Carrier in ocean.", 0x14D, 1 },
	{ "FLAG_AMY_MR_ISLANDDOOR", "Amy: Unused, but works. Opens Angel Island cave door.", 0x14E, 1 },
	{ "FLAG_AMY_MR_TRUCK", "Amy: Spawns Jungle minecart. ", 0x14F, 1 },
	{ "FLAG_AMY_MR_APPEAR_FINALEGG", "Amy: Spawns Eggman's base in the jungle.", 0x150, 1 },
	{ "FLAG_AMY_MR_ENTRANCE_FINALEGG", "Amy: Opens Final Egg.", 0x151, 1 },
	{ "FLAG_AMY_MR_FIGHTERSFEATHER", "Amy: Has Warrior Feather.", 0x152, 1 },
	{ "FLAG_AMY_TRAIN", "Amy: Enables SS <-> MR Train.", 0x153, 1 },
	{ "FLAG_AMY_BOAT", "Amy: Enables SS <-> EC Boat.", 0x154, 1 },
	{ "FLAG_AMY_RAFT", "Amy: Enables MR <-> EC Raft.", 0x155, 1 },
	{ "FLAG_AMY_CLEAR_TWINKLEPARK", "Amy: Cleared Twinkle Park.", 0x156, 1 },
	{ "FLAG_AMY_CLEAR_HOTSHELTER", "Amy: Cleared Hot Shelter.", 0x157, 1 },
	{ "FLAG_AMY_CLEAR_FINALEGG", "Amy: Cleared Final Egg.", 0x158, 1 },
	{ "FLAG_AMY_CLEAR_ZERO", "Amy: Cleared ZERO.", 0x159, 1 },
	{ "FLAG_AMY_EC_1STVISIT", "Amy: Unused.", 0x15A, 1 },
	{ "FLAG_AMY_ARRIVE_IN_SS", "Amy: Enables SS Chao Garden warp.", 0x15B, 1 },
	{ "FLAG_AMY_ARRIVE_IN_EC", "Amy: Enables EC Chao Garden warp.", 0x15C, 1 },
	{ "FLAG_AMY_ARRIVE_IN_MR", "Amy: Enables MR Chao Garden warp.", 0x15D, 1 },
	{ "FLAG_AMY_EC_LONGHAMMER", "Amy: Has Long Hammer.", 0x15E, 1 },
	{ "FLAG_E102_SS_HOTEL_FRONT", "E102: Opens the hotel's front door.", 0x180, 1 },
	{ "FLAG_E102_SS_HOTEL_POOL", "E102: Opens the hotel's back door.", 0x181, 1 },
	{ "FLAG_E102_SS_STATION_FRONT", "E102: Opens the station.", 0x182, 1 },
	{ "FLAG_E102_SS_TPARK_ELEVATOR", "E102: Enables the Twinkle Park elevator.", 0x183, 1 },
	{ "FLAG_E102_SS_ENTRANCE_CIRCUIT", "E102: Enables the Twinkle Circuit door.", 0x184, 1 },
	{ "FLAG_E102_SS_CARTPASSPORT", "E102: Cart Pass exists (if SS_ENTRANCE_CIRCUIT is OFF)", 0x185, 1 },
	{ "FLAG_E102_EC_MONORAIL", "E102: Enables Monorail.", 0x186, 1 },
	{ "FLAG_E102_EC_EGGLIFT", "E102: Enables Egg Carrier spinning elevator.", 0x187, 1 },
	{ "FLAG_E102_EC_TRANSFORM", "E102: Transforms Egg Carrier.", 0x188, 1 },
	{ "FLAG_E102_EC_BOOSTER", "E102: Has Jet Booster", 0x189, 1 },
	{ "FLAG_E102_EC_TYPE3LASER", "E102: Has Laser Blaster.", 0x18A, 1 },
	{ "FLAG_E102_EC_TORNADO2_LOST", "E102: Removes crashed Tornado 2. (if EC_SINK is OFF)", 0x18B, 1 },
	{ "FLAG_E102_EC_SINK", "E102: Egg Carrier in ocean.", 0x18C, 1 },
	{ "FLAG_E102_EC_HOTSHELTER", "E102: Opens Hot Shelter.", 0x18D, 1 },
	{ "FLAG_E102_MR_GOODSCORE", "E102: Unused.", 0x18E, 1 },
	{ "FLAG_E102_MR_FREEPASS", "E102: Opens exit to Mystic Ruins jungle base.", 0x18F, 1 },
	{ "FLAG_E102_MR_WINDYSTONE", "E102: Wind Stone exists (if MR_ENTRANCE_WINDY is OFF)", 0x190, 1 },
	{ "FLAG_E102_MR_ENTRANCE_WINDY", "E102: Opens Windy Valley.", 0x191, 1 },
	{ "FLAG_E102_MR_WESTROCK", "E102: Opens Angel Island cave.", 0x192, 1 },
	{ "FLAG_E102_MR_ENTRANCE_MOUNTAIN", "E102: Opens Red Mountain gate.", 0x193, 1 },
	{ "FLAG_E102_MR_ISLANDDOOR", "E102: Opens Angel Island cave door.", 0x194, 1 },
	{ "FLAG_E102_MR_TRUCK", "E102: Spawns Jungle minecart.", 0x195, 1 },
	{ "FLAG_E102_MR_APPEAR_FINALEGG", "E102: Spawns Eggman's base in the jungle.", 0x196, 1 },
	{ "FLAG_E102_TRAIN", "E102: Enables SS <-> MR Train.", 0x197, 1 },
	{ "FLAG_E102_BOAT", "E102: Enables SS <-> EC Boat.", 0x198, 1 },
	{ "FLAG_E102_RAFT", "E102: Enables MR <-> EC Raft.", 0x199, 1 },
	{ "FLAG_E102_MBOSS_SONIC", "E102: Cleared Sonic. (Boss) ", 0x19A, 1 },
	{ "FLAG_E102_CLEAR_BEACH", "E102: Cleared Emerald Coast.", 0x19B, 1 },
	{ "FLAG_E102_CLEAR_WINDY", "E102: Cleared Windy Valley.", 0x19C, 1 },
	{ "FLAG_E102_CLEAR_MOUNTAIN", "E102: Cleared Red Mountain.", 0x19D, 1 },
	{ "FLAG_E102_CLEAR_HOTSHELTER", "E102: Cleared Hot Shelter.", 0x19E, 1 },
	{ "FLAG_E102_CLEAR_FINALEGG", "E102: Cleared Final Egg.", 0x19F, 1 },
	{ "FLAG_E102_CLEAR_E101", "E102: Cleared E101.", 0x1A0, 1 },
	{ "FLAG_E102_CLEAR_E101R", "E102: Cleared E101mkII.", 0x1A1, 1 },
	{ "FLAG_E102_ARRIVE_IN_SS", "E102: Enables SS Chao Garden warp.", 0x1A2, 1 },
	{ "FLAG_E102_ARRIVE_IN_EC", "E102: Enables EC Chao Garden warp.", 0x1A3, 1 },
	{ "FLAG_E102_ARRIVE_IN_MR", "E102: Enables MR Chao Garden warp.", 0x1A4, 1 },
	{ "FLAG_BIG_SS_HOTEL_FRONT", "Big: Opens the hotel's front door.", 0x1C0, 1 },
	{ "FLAG_BIG_SS_HOTEL_BACK", "Big: Opens the hotel's back door.", 0x1C1, 1 },
	{ "FLAG_BIG_SS_TPARK_ELEVATOR", "Big: Enables the Twinkle Park elevator.", 0x1C2, 1 },
	{ "FLAG_BIG_SS_ENTRANCE_CIRCUIT", "Big: Enables the Twinkle Circuit door.", 0x1C3, 1 },
	{ "FLAG_BIG_SS_STATION_FRONT", "Big: Opens the station.", 0x1C4, 1 },
	{ "FLAG_BIG_SS_ICESTONE", "Big: Ice Stone exists (if MR_ENTRANCE_ICECAP is OFF)", 0x1C5, 1 },
	{ "FLAG_BIG_SS_HOTEL_POOL", "Big: Opens the hotel's pool door. ", 0x1C6, 1 },
	{ "FLAG_BIG_EC_MONORAIL", "Big: Enables Monorail.", 0x1C7, 1 },
	{ "FLAG_BIG_EC_EGGLIFT", "Big: Enables Egg Carrier spinning elevator.", 0x1C8, 1 },
	{ "FLAG_BIG_EC_TRANSFORM", "Big: Transforms Egg Carrier.", 0x1C9, 1 },
	{ "FLAG_BIG_EC_TORNADO2_LOST", "Big: Removes crashed Tornado 2. (if EC_SINK is OFF)", 0x1CA, 1 },
	{ "FLAG_BIG_EC_SINK", "Big: Egg Carrier in ocean.", 0x1CB, 1 },
	{ "FLAG_BIG_MR_WESTROCK", "Big: Opens Angel Island cave.", 0x1CC, 1 },
	{ "FLAG_BIG_MR_ICESTONE", "Big: Ice Stone exists (if MR_ENTRANCE_ICECAP is OFF)", 0x1CD, 1 },
	{ "FLAG_BIG_MR_ENTRANCE_ICECAP", "Big: Opens Icecap door.", 0x1CE, 1 },
	{ "FLAG_BIG_MR_TRUCK", "Big: Spawns Jungle minecart.", 0x1CF, 1 },
	{ "FLAG_BIG_MR_LIFEBELT", "Big: Has Life Belt.", 0x1D0, 1 },
	{ "FLAG_BIG_MR_POWERROD", "Big: Has Power Rod.", 0x1D1, 1 },
	{ "FLAG_BIG_TRAIN", "Big: Enables SS <-> MR Train.", 0x1D2, 1 },
	{ "FLAG_BIG_BOAT", "Big: Enables SS <-> EC Boat.", 0x1D3, 1 },
	{ "FLAG_BIG_RAFT", "Big: Enables MR <-> EC Raft.", 0x1D4, 1 },
	{ "FLAG_BIG_CLEAR_BEACH", "Big: Cleared Emerald Coast.", 0x1D5, 1 },
	{ "FLAG_BIG_CLEAR_HOTSHELTER", "Big: Cleared Hot Shelter.", 0x1D6, 1 },
	{ "FLAG_BIG_CLEAR_TWINKLEPARK", "Big: Cleared Twinkle Park.", 0x1D7, 1 },
	{ "FLAG_BIG_CLEAR_TWINKLEPARK", "Big: Cleared Twinkle Park.", 0x1D7, 1 },
	{ "FLAG_BIG_CLEAR_SNOW", "Big: Cleared Icecap.", 0x1D8, 1 },
	{ "FLAG_BIG_CLEAR_CHAOS6", "Big: Cleared Chaos 6.", 0x1D9, 1 },
	{ "FLAG_BIG_ARRIVE_IN_SS", "Big: Enables SS Chao Garden warp.", 0x1DA, 1 },
	{ "FLAG_BIG_ARRIVE_IN_EC", "Big: Enables EC Chao Garden warp.", 0x1DB, 1 },
	{ "FLAG_BIG_ARRIVE_IN_MR", "Big: Enables MR Chao Garden warp.", 0x1DC, 1 },
	{ "FLAG_BIG_ARRIVE_IN_SEWER", "Big: Unused.", 0x1DD, 1 },
	{ "FLAG_BIG_RUAR_BLUE", "Big: Lure Upgrade (Egg Carrier Prison)", 0x1DE, 1 },
	{ "FLAG_BIG_RUAR_GOLD", "Big: Lure Upgrade (Icecap)", 0x1DF, 1 },
	{ "FLAG_BIG_RUAR_NORMAL", "Big: Lure Upgrade (Station Square)", 0x1E0, 1 },
	{ "FLAG_BIG_RUAR_RED", "Big: Lure Upgrade (Mystic Ruins Jungle)", 0x1E1, 1 },
	{ "FLAG_BIG_RUAR_SILVER", "Big: Unused. Affects lure model in upgrade rings due to an oversight.", 0x1E2, 1 },
	{ "FLAG_EVENT_HINTBOX_DISP", "Enables Hint Boxes in events. (Everywhere if enabled in config)", 0x1E3, 1 },
	{ "FLAG_EVENT_NPC_DISP", "Enables NPCs in events. (Everywhere if enabled in config)", 0x1E4, 1 },
	{ "FLAG_ALL_LAST", "", 0x1E5, 1 },
	{ NULL }
};

static fieldflag* flagList = adventureFlags;

DataArray(char, evExecFlag, 0x3B18608, 512);

const char* timeofDayStrs[] = {
	"OFF",
	"DAY",
	"EVENING",
	"NIGHT",
};

/// <summary>
/// HUD for SeqVars Editor
/// </summary>
void displaySeqVarsInfo(Sint32 col, Sint32 pno) {
	njPrintC(NJM_LOCATION(1, col++), "Mode: Field Flag Editor");
	col++;

	if (isEventFlags) {
		njPrintC(NJM_LOCATION(2, col++), "Toggle Event Flags.");
		njPrintC(NJM_LOCATION(2, col++), "Z Button (RB) = BACK");
		njPrintC(NJM_LOCATION(2, col++), "Y Button + Left/Right = Change List");
		col++;
		njPrintC(NJM_LOCATION(2, col++), "The game uses this list to track which events you've seen.");
		njPrintC(NJM_LOCATION(2, col++), "It's used primarily for choosing which NPCs and hints load.");
	}
	else {
		njPrintC(NJM_LOCATION(2, col++), "Toggle Adventure Field Flags.");
		njPrintC(NJM_LOCATION(2, col++), "Z Button (RB) = BACK");
		njPrintC(NJM_LOCATION(2, col++), "Y Button + Left/Right = Change List");
		col++;
		njPrintC(NJM_LOCATION(2, col++), "TIPS:");
		njPrintC(NJM_LOCATION(2, col++), "- The game overrides some SS/MR/EC flags.");
		njPrintC(NJM_LOCATION(2, col++), "- Clear flags lock entrances until the next stage's is on.");
		njPrintC(NJM_LOCATION(2, col++), "- Some flags need areas reloaded before having an effect.");
	}
	col++;
	Sint32 colArrow = col + cursor;
	njPrintC(NJM_LOCATION(1, colArrow), ">");

	maxID = 0;

	for (Sint32 i = baseID; i < baseID + 10; ++i) {
		if (flagList[i].name == NULL) {
			break;
		}

		if (isEventFlags) {
			njPrint(NJM_LOCATION(2, col++), "[%d]%s: %s", flagList[i].flagID, flagList[i].name, evExecFlag[flagList[i].flagID] ? "ON" : "OFF");
		}
		else {
			if (i == 0)
				njPrint(NJM_LOCATION(2, col++), "%s: %s", flagList[i].name, timeofDayStrs[timeOfDay_forced]);
			else if (flagList[i].maxVal > 1)
				njPrint(NJM_LOCATION(2, col++), "[%d]%s: %d", flagList[i].flagID, flagList[i].name, seqVars[flagList[i].flagID]);
			else
				njPrint(NJM_LOCATION(2, col++), "[%d]%s: %s", flagList[i].flagID, flagList[i].name, seqVars[flagList[i].flagID] ? "ON" : "OFF");
		}
		++maxID;
	}

	col++;
	njPrint(NJM_LOCATION(2, col++), "%s", flagList[baseID + cursor].desc);

}

///Animation Player Main Function
void doSeqVarsEditor(Sint32 pno) {
	//Selection Arrow
	Uint32 isPressorOn = per[0]->press;
	if (per[0]->on & Buttons_Y) {
		isPressorOn = per[0]->on & ~Buttons_Y;

		//Swap between field and event flags
		if (per[0]->press & Buttons_Right) {
			isEventFlags = TRUE;
			flagList = eventFlags;

			if (flagList != eventFlags) {
				cursor = baseID;
				flagList = eventFlags;
			}
		}
		else if (per[0]->press & Buttons_Left) {
			isEventFlags = FALSE;

			if (flagList != adventureFlags) {
				cursor = baseID;
				flagList = adventureFlags;
			}
		}
	}

	if (per[0]->press & Buttons_Z) {
		maker_mode = 0;
		return;
	}

	switch (isPressorOn) {
	case Buttons_Right:
		if (!(per[0]->on & Buttons_Y)) {
			if (isEventFlags) {
				evExecFlag[flagList[baseID + cursor].flagID] = evExecFlag[flagList[baseID + cursor].flagID] ? FALSE : TRUE;
			}
			else if (baseID + cursor == 0 && !isEventFlags) {
				timeOfDay_forced = NJM_MIN(3, timeOfDay_forced + 1);
			}
			else {
				++seqVars[flagList[baseID + cursor].flagID];

				if (seqVars[flagList[baseID + cursor].flagID] > flagList[baseID + cursor].maxVal) {
					seqVars[flagList[baseID + cursor].flagID] = 0;
				}
			}
		}
		break;
	case Buttons_Left:
		if (!(per[0]->on & Buttons_Y)) {
			if (isEventFlags) {
				evExecFlag[flagList[baseID + cursor].flagID] = evExecFlag[flagList[baseID + cursor].flagID] ? FALSE : TRUE;
			}
			else if (baseID + cursor == 0) {
				timeOfDay_forced = NJM_MAX(0, timeOfDay_forced - 1);
			}
			else {
				--seqVars[flagList[baseID + cursor].flagID];
				//Needed because tails' tornado 2 tunnel is weird and I don't want to code in skipping over IDs

				if (seqVars[flagList[baseID + cursor].flagID] < 0) {
					seqVars[flagList[baseID + cursor].flagID] = flagList[baseID + cursor].maxVal;
				}
			}
		}
		break;
	case Buttons_Up:
		
		if (cursor - 1 < 0 && baseID > 0) {
			--baseID;
		}
		else {
			cursor = NJM_MAX(0, cursor - 1);
		}
		break;
	case Buttons_Down:
		
		if (cursor + 1 == 10 && flagList[baseID + 10].name != NULL) {
			++baseID;
		}
		else {
			cursor = NJM_MIN(maxID - 1, cursor + 1);
		}

		break;
	}
}