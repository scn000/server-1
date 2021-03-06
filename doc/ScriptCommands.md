Database script processing
==========================
In addition to *EventAI*, *mangos* provides script processing for various
types of game content. These scripts can be executed when creatures move or die,
when events are executed, and game object spawns/templates are used, on gossip
menu selections, when starting and completing quests, and when casting spells.

Database structure
------------------
All script tables share a common structure, to store parameterized commands for
the various types of game content.

`id` column
-----------
* `dbscripts_on_creature_death`: Creature entry
* `dbscripts_on_creature_movement`: DB project self defined id
* `dbscripts_on_event`: Event id. Several sources: spell effect 61, taxi/transport nodes, game object_template data
* `dbscripts_on_go_use`: Game object guid
* `dbscripts_on_go_template_use`: Game object entry
* `dbscripts_on_gossip`: DB project self defined id
* `dbscripts_on_quest_end`: DB project self defined id (generally quest entry)
* `dbscripts_on_quest_start`: DB project self defined id (generally quest entry)
* `dbscripts_on_spell`: Spell id

`delay` column
--------------
Delay in seconds. Defines the order in which steps are executed.

`command` column
----------------
The action to execute.

`datalong` columns
------------------
2 multipurpose fields, storing raw data as unsigned integer values.

`buddy_entry` column
--------------------
1 field to store the entry of a "buddy". Depending on the command used this can
can point to a game object template or and creature template entry.

`search_radius` column
----------------------
Range in which the buddy defined in `buddy_entry` will be searched. In case of
`SCRIPT_FLAG_BUDDY_BY_GUID` this field is the buddy's guid!

`data_flags` column
-------------------
Field which holds a combination of these flags:

Value | Name                            | Notes
----- | ------------------------------- | ------------------------------------
1     | SCRIPT_FLAG_BUDDY_AS_TARGET     |
2     | SCRIPT_FLAG_REVERSE_DIRECTION   |
4     | SCRIPT_FLAG_SOURCE_TARGETS_SELF |
8     | SCRIPT_FLAG_COMMAND_ADDITIONAL  | (Only for some commands possible)
16    | SCRIPT_FLAG_BUDDY_BY_GUID       | (Interpret search_radius as buddy's guid)
32    | SCRIPT_FLAG_BUDDY_IS_PET        | (Do not search for an NPC, but for a pet)

`dataint` columns
-----------------
4 multipurpose fields, storing raw data as signed integer values.

*Note*: used for text ids SCRIPT_COMMAND_TALK (0),
      for emote ids in SCRIPT_COMMAND_EMOTE (1),
      for spell ids in SCRIPT_COMMAND_CAST_SPELL (15)
      and as waittime with SCRIPT_COMMAND_TERMINATE_SCRIPT (31)

`x`, `y`, `z` and `o` columns
-----------------------------
Map coordinates for commands that require coordinates

Origin of script and source/target in scripts
---------------------------------------------

* `dbscripts_on_creature_death`: Creature death.
  Source: creature. Target: Unit (player/creature)
* `dbscripts_on_creature_movement`: `creature_movement` and `creature_movement_template`.
  Source: creature. Target: creature
* `dbscripts_on_event`: Flight path with source: player and target: player,
  transport path with source: transport game object and target transport game
  object, `gameobject_template` with source: user (player/creature) and target:
  game object, spells having effect 61 with source: caster and target: target.
* `dbscripts_on_go_use`, `dbscripts_on_go_template_use`: game object use with
  source: user: and target: game object.
* `dbscripts_on_gossip`: `gossip_menu_option`, `gossip_menu` with source: creature
  and target: player in case of NPC-Gossip, or with source: player and target:
  game object in case of GO-Gossip.
* `dbscripts_on_quest_end`: `quest_template` with source: quest taker (creature/GO)
  and target: player.
* `dbscripts_on_quest_start`: `quest_template` with source: quest giver (creature/GO)
  and target: player.
* `dbscripts_on_spell`: spell with effect 77(SCRIPT_EFFECT), spells with effect 3
  (DUMMY), spells with effect 64 (TRIGGER_SPELL, with non-existing triggered spell)
  having source: caster: and target: target of spell (Unit).

Buddy concept
-------------
Commands except the ones requiring a player (like KILL_CREDIT) have support
for the buddy concept. This means that if an entry for `buddy_entry` is
provided, aside from source and target as listed above also a "buddy" is
available.

Which one on the three (`originalSource`, `originalTarget`, `buddy`) will
be used in the command, depends on the `data_flags` settings.

Note that some commands (like EMOTE) use only the resulting source for an
action.

Possible combinations of the flags:

* `SCRIPT_FLAG_BUDDY_AS_TARGET`: 0x01
* `SCRIPT_FLAG_REVERSE_DIRECTION`: 0x02
* `SCRIPT_FLAG_SOURCE_TARGETS_SELF`: 0x04

are

* 0: originalSource / buddyIfProvided  ->  originalTarget
* 1: originalSource  ->  buddy
* 2: originalTarget  ->  originalSource / buddyIfProvided
* 3: buddy  ->  originalSource
* 4: originalSource / buddyIfProvided  ->  originalSource / buddyIfProvided
* 5: originalSource  ->  originalSource
* 6: originalTarget  ->  originalTarget
* 7: buddy  ->  buddy

where `A -> B` means that the command is executed from A with B as target.

Commands and their parameters
-----------------------------

ID | Name                                   | Parameters
-- | -------------------------------------- | -------------------------------------
0  | SCRIPT_COMMAND_TALK                    | resultingSource = WorldObject, resultingTarget = Unit/none, `dataint` = text entry from db_script_string -table. `dataint2`-`dataint4` optionally, for random selection of text
1  | SCRIPT_COMMAND_EMOTE                   | resultingSource = Unit, resultingTarget = Unit/none, `datalong` = emote_id, dataint1-dataint4 = optionally for random selection of emote
2  | SCRIPT_COMMAND_FIELD_SET               | source = any, `datalong` = field_id, `datalong2` = field value
3  | SCRIPT_COMMAND_MOVE_TO                 | resultingSource = Creature. If position is very near to current position, or x=y=z=0, then only orientation is changed. `datalong2` = travel_speed*100 (use 0 for creature default movement). `data_flags` & SCRIPT_FLAG_COMMAND_ADDITIONAL: teleport unit to position `x`/`y`/`z`/`o`
4  | SCRIPT_COMMAND_FLAG_SET                | source = any. `datalong` = field_id, `datalong2` = bit mask
5  | SCRIPT_COMMAND_FLAG_REMOVE             | source = any. `datalong` = field_id, `datalong2` = bit mask
6  | SCRIPT_COMMAND_TELEPORT_TO             | source or target with Player. `datalong` = map_id, x/y/z
7  | SCRIPT_COMMAND_QUEST_EXPLORED          | one from source or target must be Player, another GO/Creature. `datalong` = quest_id, `datalong2` = distance or 0
8  | SCRIPT_COMMAND_KILL_CREDIT             | source or target with Player. `datalong` = creature entry, or 0; If 0 the entry of the creature source or target is used, `datalong2` = bool (0=personal credit, 1=group credit)
9  | SCRIPT_COMMAND_RESPAWN_GAMEOBJECT      | source = any, target = any. `datalong`=db_guid (can be skipped for buddy), `datalong2` = despawn_delay
10 | SCRIPT_COMMAND_TEMP_SUMMON_CREATURE    | source = any, target = any. `datalong` = creature entry, `datalong2` = despawn_delay, `data_flags` & SCRIPT_FLAG_COMMAND_ADDITIONAL: summon as active object
11 | SCRIPT_COMMAND_OPEN_DOOR               | source = any. `datalong` = db_guid (can be skipped for buddy), `datalong2` = reset_delay
12 | SCRIPT_COMMAND_CLOSE_DOOR              | source = any. `datalong` = db_guid (can be skipped for buddy), `datalong2` = reset_delay
13 | SCRIPT_COMMAND_ACTIVATE_OBJECT         | source = unit, target=GO.
14 | SCRIPT_COMMAND_REMOVE_AURA             | resultingSource = Unit. `datalong` = spell_id
15 | SCRIPT_COMMAND_CAST_SPELL              | resultingSource = Unit, cast spell at resultingTarget = Unit. `datalong` = spell id, `dataint1`-`dataint4` optional. If some of these are set to a spell id, a random spell out of datalong, datint1, ..,dataintX is cast., `data_flags` & SCRIPT_FLAG_COMMAND_ADDITIONAL: cast triggered
16 | SCRIPT_COMMAND_PLAY_SOUND              | source = any object, target=any/player. `datalong` = sound_id, `datalong2` (bit mask: 0/1=target-player, 0/2=with distance dependent, 0/4=map wide, 0/8=zone wide; so 1|2 = 3 is target with distance dependent)
17 | SCRIPT_COMMAND_CREATE_ITEM             | source or target must be player. `datalong` = item entry, `datalong2` = amount
18 | SCRIPT_COMMAND_DESPAWN_SELF            | resultingSource = Creature. `datalong` = despawn delay
19 | SCRIPT_COMMAND_PLAY_MOVIE              | target can only be a player. `datalong` = movie id
20 | SCRIPT_COMMAND_MOVEMENT                | resultingSource = Creature. `datalong` = MovementType (0:idle, 1:random or 2:waypoint), `datalong2` = wanderDistance (for random movement), `data_flags` & SCRIPT_FLAG_COMMAND_ADDITIONAL: RandomMovement around current position
21 | SCRIPT_COMMAND_SET_ACTIVEOBJECT        | resultingSource = Creature. `datalong` = bool 0=off, 1=on
22 | SCRIPT_COMMAND_SET_FACTION             | resultingSource = Creature. `datalong` = factionId OR 0 to restore original faction from creature_template, `datalong2` = enum TemporaryFactionFlags
23 | SCRIPT_COMMAND_MORPH_TO_ENTRY_OR_MODEL | resultingSource = Creature. `datalong` = creature entry/modelid (depend on data_flags) OR 0 to demorph, `data_flags` & SCRIPT_FLAG_COMMAND_ADDITIONAL: use datalong value as modelid explicit
24 | SCRIPT_COMMAND_MOUNT_TO_ENTRY_OR_MODEL | resultingSource = Creature. `datalong` = creature entry/modelid (depend on data_flags) OR 0 to dismount, `data_flags` & SCRIPT_FLAG_COMMAND_ADDITIONAL: use datalong value as modelid explicit
25 | SCRIPT_COMMAND_SET_RUN                 | resultingSource = Creature. `datalong` = bool 0=off, 1=on
26 | SCRIPT_COMMAND_ATTACK_START            | resultingSource = Creature, resultingTarget = Unit.
27 | SCRIPT_COMMAND_GO_LOCK_STATE           | resultingSource = GO. `datalong` = flag_go_lock = 0x01, flag_go_unlock = 0x02, flag_go_nonInteract = 0x04, flag_go_interact = 0x08
28 | SCRIPT_COMMAND_STAND_STATE             | resultingSource = Creature. `datalong` = stand state (enum UnitStandStateType)
29 | SCRIPT_COMMAND_MODIFY_NPC_FLAGS        | resultingSource = Creature. `datalong` = NPCFlags, `datalong2` = 0x00=toggle, 0x01=add, 0x02=remove
30 | SCRIPT_COMMAND_SEND_TAXI_PATH          | resultingTarget or Source must be Player. `datalong` = taxi path id
31 | SCRIPT_COMMAND_TERMINATE_SCRIPT        | `datalong` = search for npc entry if provided, `datalong2` = search distance, `!(data_flags & SCRIPT_FLAG_COMMAND_ADDITIONAL)`: if npc not alive found, terminate script, `data_flags & SCRIPT_FLAG_COMMAND_ADDITIONAL`:  if npc alive found, terminate script, `dataint` = change of waittime (MILLISECONDS) of a current waypoint movement type (negative values will decrease time)
32 | SCRIPT_COMMAND_PAUSE_WAYPOINTS         | resultingSource must be Creature. `datalong` = 0/1 unpause/pause waypoint movement
33 | SCRIPT_COMMAND_RESERVED_1              | reserved for 3.x and later. Do not use!
34 | SCRIPT_COMMAND_TERMINATE_COND          | `datalong` = condition_id, `datalong2` = fail-quest (if provided this quest will be failed for a player), `!(data_flags & SCRIPT_FLAG_COMMAND_ADDITIONAL)`: terminate when condition is true, `data_flags & SCRIPT_FLAG_COMMAND_ADDITIONAL`:  terminate when condition is false
35 | SCRIPT_COMMAND_SEND_AI_EVENT_AROUND    | resultingSource = Creature, resultingTarget = Unit, datalong = AIEventType - limited only to EventAI supported events, datalong2 = radius
36 | SCRIPT_COMMAND_SET_FACING              |  Turn resultingSource towards resultingTarget
                                            | * resultingSource = Creature, resultingTarget WorldObject
                                            | * data_flags & SCRIPT_FLAG_COMMAND_ADDITIONAL also set TargetGuid of resultingSource to resultingTarget.
                                            |   In this case resultingTarget MUST be Creature/ Player
                                            | * datalong != 0 Reset TargetGuid, Reset orientation

37 | SCRIPT_COMMAND_MOVE_DYNAMIC            | Move resultingSource to a random point around resultingTarget or to resultingTarget
                                            | * resultingSource = Creature, resultingTarget Worldobject.
                                            | * datalong = 0:  Move resultingSource towards resultingTarget
                                            | * datalong != 0: Move resultingSource to a random point between datalong2..datalong around resultingTarget.
                                            |   orientation != 0: Obtain a random point around resultingTarget in direction of orientation
                                            | * data_flags & SCRIPT_FLAG_COMMAND_ADDITIONAL Obtain a point in direction of resTarget->GetOrientation + orientation
                                            |   for resTarget == resSource and orientation == 0 this will mean resSource moving forward
38 | SCRIPT_COMMAND_SEND_MAIL               | Send a mail from resSource  to resTarget
                                            | * resultingSource = Creature OR NULL, resTarget must be Player
                                            | * datalong = mailTemplateId
                                            | * datalong2: AlternativeSenderEntry. Use as sender-Entry of the sent mail
                                            | * dataint1: Delay (>= 0) in Seconds

TemporaryFactionFlags
---------------------
* `TEMPFACTION_NONE`: 0x00, when no flag is used in temporary faction change, faction will be persistent. It will then require manual change back to default/another faction when changed once
* `TEMPFACTION_RESTORE_RESPAWN`: 0x01, default faction will be restored at respawn
* `TEMPFACTION_RESTORE_COMBAT_STOP`: 0x02, ... at CombatStop() (happens at creature death, at evade or custom script among others)
* `TEMPFACTION_RESTORE_REACH_HOME`: 0x04, ... at reaching home in home movement (evade), if not already done at CombatStop()

The next flags allow to remove unit_flags combined with a faction change (also these flags will be reapplied when the faction is changed back)

* `TEMPFACTION_TOGGLE_NON_ATTACKABLE`: 0x08, remove UNIT_FLAG_NON_ATTACKABLE(0x02) when faction is changed (reapply when temp-faction is removed)
* `TEMPFACTION_TOGGLE_OOC_NOT_ATTACK`: 0x10, remove UNIT_FLAG_OOC_NOT_ATTACKABLE(0x100) when faction is changed (reapply when temp-faction is removed)
* `TEMPFACTION_TOGGLE_PASSIVE`       : 0x20, remove UNIT_FLAG_PASSIVE(0x200) when faction is changed (reapply when temp-faction is removed)
* `TEMPFACTION_TOGGLE_PACIFIED`      : 0x40, remove UNIT_FLAG_PACIFIED(0x20000) when faction is changed (reapply when temp-faction is removed)
* `TEMPFACTION_TOGGLE_NOT_SELECTABLE`: 0x80, remove UNIT_FLAG_NOT_SELECTABLE(0x2000000) when faction is changed (reapply when temp-faction is removed)
