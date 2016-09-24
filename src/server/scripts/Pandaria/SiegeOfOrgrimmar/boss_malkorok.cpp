#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "MapManager.h"
#include "Spell.h"
#include "Vehicle.h"
#include "Cell.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "CreatureTextMgr.h"
#include "ScriptedCreature.h"
#include "ScriptPCH.h"
#include "ScriptedEscortAI.h"

enum eBosses
{
	BOSS_MALKOROK,
};

enum eSpells
{
	SPELL_ANCIENT_MIASMA	= 142861, 
	SPELL_ARCING_SMASH		= 142815, 
	SPELL_SEISMIC_SLAM		= 142851, 
	SPELL_DISPLACED_ENERGY	= 142913, 
	SPELL_EXPEL_MIASMA		= 143199, 
	SPELL_BREATH_OF_YSHAARJ = 142842, 
	SPELL_ERADICATE			= 143916,  
	SPELL_BLOOD_RAGE		= 142879,
    SPELL_IMPLODING_ENERGY	= 142986,
	SPELL_FATAL_STRIKE		= 142990
};

enum eEvents
{
	EVENT_ARCING_SMASH				= 1,
	EVENT_SEISMIC_SLAM				= 2,
	EVENT_DISPLACED_ENERGY			= 3,
	EVENT_ERADICATE					= 4,
	EVENT_BREATH_OF_YSHARRJ			= 5,
	EVENT_EXPEL_MIASMA				= 6,
	EVENT_ARCING_SMASH_TARGET_SPAWN = 7,
	EVENT_AGRESSIVE					= 8,
	EVENT_BLOOD_RAGE				= 9,
	EVENT_PHASE1					= 10,
	EVENT_IMPLODING_ENERY			= 11,
};

enum Phases
{
	PHASE_ONE = 1,
	PHASE_TWO = 2,
};

enum eCreatures
{
	CREATURE_MALKOROK	  = 71454,
	CREATURE_ARCING_SMASH = 71455
};

enum eTexts
{
	MALKOROK_INTRO					= 1,
	MALKOROK_AGGRO					= 2,
	MALKOROK_ARCING_SMASH_1			= 3,
	MALKOROK_ARCING_SMASH_2			= 4,
	MALKOROK_ARCING_SMASH_3			= 5,
	MALKOROK_BREATH_OF_YSHAARJ		= 6, // 0 or 1 in database
	MALKOROK_BLOOD_RAGE_1			= 7,
	MALKOROK_BLOOD_RAGE_2			= 8,
	MALKOROK_BERSERK				= 9,
	MALKOROK_WIPE					= 10,
	MALKOROK_DEATH					= 11,
};

class boss_malkorok : public CreatureScript
{
public:
	boss_malkorok() : CreatureScript("boss_malkorok") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new boss_malkorok_AI(creature);
	}

	struct boss_malkorok_AI : public BossAI
	{
		boss_malkorok_AI(Creature* creature) : BossAI(creature, BOSS_MALKOROK)
		{
		
		}

		void Reset()
		{
			_Reset();
			me->SetReactState(REACT_AGGRESSIVE);
			events.Reset();
			me->setFaction(16);
			me->setPowerType(POWER_RAGE);
			me->SetMaxPower(POWER_RAGE, 100);
			events.SetPhase(PHASE_ONE);
		}

		void JustDied(Unit* /*killer*/)
		{
			Talk(MALKOROK_DEATH);
			std::list<Player*> pl_list;
			me->GetPlayerListInGrid(pl_list, 500.0f);
			for (auto itr : pl_list)
			{
				if (itr->HasAura(SPELL_ANCIENT_MIASMA))
					itr->RemoveAura(SPELL_ANCIENT_MIASMA);
			}
		}

		void KilledUnit(Unit* u)
		{
		}


		void EnterCombat(Unit* unit)
		{
			DoCastToAllHostilePlayers(SPELL_ANCIENT_MIASMA);           
			Talk(MALKOROK_AGGRO);
			std::list<Player*> pl_list;
			me->GetPlayerListInGrid(pl_list, 500.0f);

			for (auto itr : pl_list)
			{
				if (!itr->HasAura(SPELL_ANCIENT_MIASMA))
					me->AddAura(SPELL_ANCIENT_MIASMA, itr);
			}

			events.SetPhase(PHASE_ONE);
			events.ScheduleEvent(EVENT_SEISMIC_SLAM, 5000, PHASE_ONE);
			events.ScheduleEvent(EVENT_ARCING_SMASH, 11000, PHASE_ONE);
			events.ScheduleEvent(EVENT_BREATH_OF_YSHARRJ, 68000, PHASE_ONE);
			me->AddAura(SPELL_FATAL_STRIKE, me);
		}

		void UpdateAI(const uint32 diff)
		{
			if (!UpdateVictim())
				return;

			if (me->HasUnitState(UNIT_STATE_CASTING))
				return;

			events.Update(diff);


			switch (events.ExecuteEvent())
			{
				case EVENT_ARCING_SMASH:
				{
					if (Unit* target = me->FindNearestCreature(CREATURE_ARCING_SMASH, 50.00f, true))
					{
						DoCast(target, SPELL_ARCING_SMASH);
					}

					events.ScheduleEvent(EVENT_ARCING_SMASH, 19000, PHASE_ONE);
					events.ScheduleEvent(EVENT_IMPLODING_ENERY, 10000, PHASE_ONE);
					break;
				}

				case EVENT_SEISMIC_SLAM:
				{
					if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 40.0f, true))
					{
						DoCast(target, SPELL_SEISMIC_SLAM, false);
					}

					events.ScheduleEvent(EVENT_SEISMIC_SLAM, 19500, PHASE_ONE);
					break;
				}

				case EVENT_BREATH_OF_YSHARRJ:
				{
					DoCast(me, SPELL_BREATH_OF_YSHAARJ, false);

					events.ScheduleEvent(EVENT_BREATH_OF_YSHARRJ, 70000, PHASE_ONE);
					events.ScheduleEvent(EVENT_SEISMIC_SLAM, 7500, PHASE_ONE);
					events.ScheduleEvent(EVENT_ARCING_SMASH, 14000, PHASE_ONE);
					break;
				}

				case EVENT_EXPEL_MIASMA:
				{
					DoCast(me, SPELL_EXPEL_MIASMA, false);

					events.ScheduleEvent(EVENT_SEISMIC_SLAM, 7500, PHASE_ONE);
					events.ScheduleEvent(EVENT_ARCING_SMASH, 14000, PHASE_ONE);
					events.ScheduleEvent(EVENT_BREATH_OF_YSHARRJ, 70000, PHASE_ONE);
					break;
				}

				case EVENT_ERADICATE:
				{
					DoCast(me, SPELL_ERADICATE, false);
					break;
				}

				case EVENT_ARCING_SMASH_TARGET_SPAWN:
				{
					float homeX = me->GetHomePosition().GetPositionX();
					float homeY = me->GetHomePosition().GetPositionY();
					float homeZ = me->GetHomePosition().GetPositionZ();
					me->GetMotionMaster()->MoveJump(homeX, homeY, homeZ, 40.0f, 40.0f);

					if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
					{
						float posX = target->GetPositionX();
						float posY = target->GetPositionY();
						float posZ = target->GetPositionZ();

						me->SummonCreature(CREATURE_ARCING_SMASH, posX, posY, posZ, 10.0f, TEMPSUMMON_TIMED_DESPAWN, 5000);
					}
					break;
				}

				case EVENT_BLOOD_RAGE:
				{
					events.SetPhase(PHASE_TWO);
					me->SetPower(POWER_RAGE, 100, false);
					DoCast(me, SPELL_BLOOD_RAGE);

					events.ScheduleEvent(EVENT_PHASE1, 22500);
					events.ScheduleEvent(EVENT_DISPLACED_ENERGY, 3500, PHASE_TWO);
					break;
				}

				case EVENT_IMPLODING_ENERY:
				{
					me->CastSpell(me, SPELL_IMPLODING_ENERGY);
					break;
				}

				case EVENT_PHASE1:
				{
					events.SetPhase(PHASE_ONE);
					break;
				}

				case EVENT_DISPLACED_ENERGY:
				{
					me->CastSpell(me, SPELL_DISPLACED_ENERGY);
					events.ScheduleEvent(EVENT_DISPLACED_ENERGY, 11000, PHASE_TWO);
					break;
				}
			}

			DoMeleeAttackIfReady();
		}
	};
};

void AddSC_boss_malkorok()
{
	new boss_malkorok();
}

/*
INSERT INTO creature_text VALUES
(71454, 0, 0, "Nazgrim has failed us. Kor'kron, destroy these intruders!", 14, 0, 100, 0, 0, 'MALKOROK_INTRO'),
(71454, 1, 0, "I will never fail the Warchief!", 14, 0, 100, 0, 0, soundId, 'MALKOROK_AGGRO'),
(71454, 2, 0, "Weaklings!", 14, 0, 100, soundId, 'MALKOROK_ARCING_SMASH_1'),
(71454, 2, 1, "You dare challenge the Warchief?", 14, 0, 100, 0, 0, soundId, 'MALKOROK_ARCING_SMASH_2'),
(71454, 2, 2, "Garrosh gives me strength!", 14, 0, 100, 0, 0, soundId, 'MALKOROK_ARCING_SMASH_3'),
(71454, 3, 0, "Witness the might of the True Horde!", 14, 0, 100, 0, 0, soundId, 'MALKOROK_BREATH_OF_YSHAARJ_1'),
(71454, 3, 1, "The power of the Warchief will overwhelm you!", 14, 0, 100, 0, 0, soundId, 'MALKOROK_BREATH_OF_YSHAARJ_2'),
(71454, 4, 0, "The True Horde cannot be STOPPED!", 14, 0, 100, 0, 0, soundId, 'MALKOROK_BLOOD_RAGE_1'),
(71454, 4, 1, "Stand and FACE ME!", 14, 0, 100, 0, 0, soundId, 'MALKOROK_BLOOD_RAGE_2'),
(71454, 5, 0, "The overwhelming power!", 14, 0, 100, 0, 0, soundId, 'MALKOROK_BERSERK'),
(71454, 6, 0, "The Warchief demands it!", 14, 0, 100, 0, 0, soundId, 'MALKOROK_WIPE'),
(71454, 7, 0, "To die... for the warchief... is... an... honor...", 14, 0, 100, 0, 0, soundId, 'MALKOROK_DEATH');
*/
