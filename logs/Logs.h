#include <list>
#include <map>
#include <vector>

#include "Energy.h"
#include "Logger.h"
#include "misc.h"
#include "simconst.h"

//===========================================================================
// Logs
//===========================================================================

// Global instance. Constructed/deleted by TSimulation.
extern class Logs *logs;

class Logs
{
 private:
	friend class TSimulation;

	Logs( class TSimulation *sim, proplib::Document *doc );
	virtual ~Logs();

 private:
	friend class Logger;

	// Invoked by Logger constructor.
	static void installLogger( Logger *logger );

	// Configure which events logger will receive.
	static void registerEvents( Logger *logger,
								sim::EventType eventTypes );

 private:
	typedef std::list<Logger *> LoggerList;
	typedef std::map< sim::EventType, LoggerList > EventRegistry;

	static LoggerList _installedLoggers;

	// Bitwise OR of all registered event types.
	static sim::EventType _registeredEvents;

	// Maps from a given event type to all registered logs.
	static EventRegistry _eventRegistry;
	
 public:
	//---------------------------------------------------------------------------
	// Logs::postEvent
	//
	// Route event to appropriate processEvent() method of Loggers registered for this
	// event type.
	//---------------------------------------------------------------------------
	template< typename T>
	void postEvent( const T &e )
	{
		// Check if any loggers are registered.
		if( _registeredEvents & e.getType() )
		{
			// Send event to loggers.
			LoggerList &loggers = _eventRegistry[ e.getType() ];
			itfor( LoggerList, loggers, it )
			{
				(*it)->processEvent( e );
			}
		}
	}

 private:
	//===========================================================================
	// AdamiComplexityLog
	//===========================================================================
	class AdamiComplexityLog : public FileLogger
	{
	protected:
		virtual void init( class TSimulation *sim, proplib::Document *doc );
		virtual int getMaxFileCount();
		virtual void processEvent( const sim::StepEndEvent &e );

	private:
		int _frequency;

	} _adamiComplexity;

	//===========================================================================
	// AgentPositionLog
	//===========================================================================
	class AgentPositionLog : public DataLibLogger
	{
	protected:
		virtual void init( class TSimulation *sim, proplib::Document *doc );
		virtual void processEvent( const sim::AgentBirthEvent &e );
		virtual void processEvent( const sim::AgentBodyUpdatedEvent &e );
		virtual void processEvent( const sim::AgentDeathEvent &e );

	} _agentPosition;

	//===========================================================================
	// BirthsDeathsLog
	//===========================================================================
	class BirthsDeathsLog : public FileLogger
	{
	protected:
		virtual void init( class TSimulation *sim, proplib::Document *doc );
		virtual void processEvent( const sim::AgentBirthEvent &birth );
		virtual void processEvent( const sim::AgentDeathEvent &death );

	} _birthsDeaths;

	//===========================================================================
	// CarryLog
	//===========================================================================
	class CarryLog : public DataLibLogger
	{
	protected:
		virtual void init( class TSimulation *sim, proplib::Document *doc );
		virtual void processEvent( const sim::CarryEvent &e );

	} _carry;

	//===========================================================================
	// CollisionLog
	//===========================================================================
	class CollisionLog : public DataLibLogger
	{
	protected:
		virtual void init( class TSimulation *sim, proplib::Document *doc );
		virtual void processEvent( const sim::CollisionEvent &e );

	} _collision;

	//===========================================================================
	// ContactLog
	//===========================================================================
	class ContactLog : public DataLibLogger
	{
	protected:
		virtual void init( class TSimulation *sim, proplib::Document *doc );
		virtual void processEvent( const sim::AgentContactEndEvent &e );
	private:
		void encode( const sim::AgentContactEndEvent::AgentInfo &info, char **buf );

	} _contact;

	//===========================================================================
	// EnergyLog
	//===========================================================================
	class EnergyLog : public DataLibLogger
	{
	protected:
		virtual void init( class TSimulation *sim, proplib::Document *doc );
		virtual void processEvent( const sim::EnergyEvent &e );

	} _energy;

	//===========================================================================
	// GenomeLog
	//===========================================================================
	class GenomeLog : public AbstractFileLogger
	{
	protected:
		virtual void init( class TSimulation *sim, proplib::Document *doc );
		virtual void processEvent( const sim::AgentBirthEvent &birth );
	} _genome;

	//===========================================================================
	// GenomeSubsetLog
	//===========================================================================
	class GenomeSubsetLog : public DataLibLogger
	{
	protected:
		virtual void init( class TSimulation *sim, proplib::Document *doc );
		virtual void processEvent( const sim::AgentBirthEvent &birth );

	private:
		std::vector<int> _geneIndexes;
	} _genomeSubset;

	//===========================================================================
	// LifeSpanLog
	//===========================================================================
	class LifeSpanLog : public DataLibLogger
	{
	protected:
		virtual void init( class TSimulation *sim, proplib::Document *doc );
		virtual void processEvent( const sim::AgentDeathEvent &death );
	} _lifespan;

	//===========================================================================
	// SeparationLog
	//===========================================================================
	class SeparationLog : public DataLibLogger
	{
	protected:
		virtual void init( class TSimulation *sim, proplib::Document *doc );
		virtual void processEvent( const sim::AgentBirthEvent &birth );
		virtual void processEvent( const sim::AgentContactBeginEvent &e );
		virtual void processEvent( const sim::AgentDeathEvent &death );
		virtual void processEvent( const sim::StepEndEvent &e );

	private:
		enum { Contact, All } _mode;
		std::list<class agent *> _births;
	} _separation;

};