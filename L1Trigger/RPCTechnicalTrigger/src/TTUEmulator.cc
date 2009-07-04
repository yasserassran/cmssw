// $Id: TTUEmulator.cc,v 1.11 2009/07/01 22:52:06 aosorio Exp $
// Include files 


#include <cmath>
// local
#include "L1Trigger/RPCTechnicalTrigger/interface/TTUEmulator.h"
#include "L1Trigger/RPCTechnicalTrigger/interface/TTUBasicConfig.h"
#include "L1Trigger/RPCTechnicalTrigger/interface/RBCLinkBoardGLSignal.h"
#include "L1Trigger/RPCTechnicalTrigger/interface/TTUGlobalSignal.h"

//-----------------------------------------------------------------------------
// Implementation file for class : TTUEmulator
//
// 2008-10-15 : Andres Osorio
//-----------------------------------------------------------------------------

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
TTUEmulator::TTUEmulator( int id, int mxw  ) 
{
  
  m_id        = id;
  m_maxWheels = mxw;
  
  int tmp[6]  = {1, -2, 0, 0, -1, 2};
  m_wheelIds = new int[6];
  for( int k=0; k < 6; ++k) m_wheelIds[k]=tmp[k];
  
  m_Wheels = new RPCWheel[2];
  for( int k=0; k < m_maxWheels; ++k ) 
    m_Wheels[k].setProperties( m_wheelIds[(id*2)+(k-2)] );
  
  m_ttuin = new TTUInput[2];
  
  m_trigger.reset();
  
  m_mode = 1;
  
  m_debug = false;
  
  m_line = 1;
  
}


TTUEmulator::TTUEmulator( int id, const char * rbclogic_type, const char * ttulogic_type, int mxw  ) 
{
  
  m_id        = id;
  m_maxWheels = mxw;

  int tmp[6]  = {1, -2, 0, 0, -1, 2};
  m_wheelIds = new int[6];
  for( int k=0; k < 6; ++k) m_wheelIds[k]=tmp[k];
  
  m_Wheels = new RPCWheel[2];
  for( int k=0; k < m_maxWheels; ++k ) 
    m_Wheels[k].setProperties( m_wheelIds[(id*2)+(k-2)], rbclogic_type );
  
  m_ttuin = new TTUInput[2];
  
  m_ttuconf   = dynamic_cast<TTUConfiguration*> (new TTUBasicConfig (ttulogic_type));
  
  m_trigger.reset();
  
  m_mode = 1;

  m_debug = false;

  m_line = 1;
  
}

TTUEmulator::TTUEmulator( int id, const char * f_name, const char * rbclogic_type, 
                          const char * ttulogic_type, int mxw  ) 
{
  
  m_id        = id;
  m_maxWheels = mxw;

  int tmp[6]  = {1, -2, 0, 0, -1, 2};
  m_wheelIds = new int[6];
  for( int k=0; k < 6; ++k) m_wheelIds[k]=tmp[k];
  
  m_Wheels = new RPCWheel[2];
  for( int k=0; k < m_maxWheels; ++k ) 
    m_Wheels[k].setProperties( m_wheelIds[(id*2)+(k-2)], f_name, rbclogic_type );
  
  m_ttuin = new TTUInput[2];
  
  m_ttuconf   = dynamic_cast<TTUConfiguration*> (new TTUBasicConfig (ttulogic_type));
  
  m_trigger.reset();
  
  m_mode = 1;
  
  m_debug = false;

  m_line = 1;
  
}

//=============================================================================
// Destructor
//=============================================================================
TTUEmulator::~TTUEmulator() {

  if ( m_wheelIds ) delete[] m_wheelIds;
  if ( m_Wheels   ) delete[] m_Wheels;
  if ( m_ttuin    ) delete[] m_ttuin;
  if ( m_ttuconf  ) delete m_ttuconf;
  
} 

//=============================================================================
void TTUEmulator::setSpecifications( const TTUBoardSpecs * ttuspecs, const RBCBoardSpecs * rbcspecs) 
{
  
  m_ttuconf   = dynamic_cast<TTUConfiguration*> (new TTUBasicConfig (ttuspecs));
  
  for( int k=0; k < m_maxWheels; ++k)
    m_Wheels[k].setSpecifications( rbcspecs );

  std::vector<TTUBoardSpecs::TTUBoardConfig>::const_iterator itr;
  itr = m_ttuconf->m_ttuboardspecs->m_boardspecs.begin();
  
  m_mode = (*itr).m_triggerMode;
  
}

bool TTUEmulator::initialise()
{
  bool status(false);
  for( int k=0; k < m_maxWheels; ++k)
    status = m_Wheels[k].initialise( );
  
  status = m_ttuconf->initialise( m_line );
  
  if ( !status ) { 
    if( m_debug ) std::cout << "TTUEmulator> Problem initialising the Configuration \n"; 
    return 0; };
  
  return status;
  
}

void TTUEmulator::SetLineId( int line )
{
  
  m_line = line;
    
}

void TTUEmulator::emulate() 
{
  
  //... only for testing
  for( int k=0; k < m_maxWheels; ++k ) 
    m_Wheels[k].emulate();
  
}

void TTUEmulator::processTtu( RPCInputSignal * signal ) 
{
  
  //. 
  int bx(0);
  bool trg(false); 
  
  m_trigger.reset();
  m_triggerBx.clear();
  
  std::vector<int> bxVec;
  std::vector<int>::iterator bxItr;
  std::map<int,RBCInput*> * linkboardin;
  std::map<int,RBCInput*>::iterator inItr;
  
  linkboardin = dynamic_cast<RBCLinkBoardGLSignal*>( signal )->m_linkboardin;
  
  for( inItr = (*linkboardin).begin(); inItr != (*linkboardin).end(); ++inItr) 
  {
    
    if ( (*inItr).first < 0 ) bx = (int) ceil( (*inItr).first / 1000000.0 );
    else bx = (int) floor( (*inItr).first / 1000000.0 );
    bxVec.push_back(bx);
    
  }
  
  bxItr = unique (bxVec.begin(), bxVec.end());
  bxVec.resize(bxItr - bxVec.begin());
  
  for ( bxItr = bxVec.begin(); bxItr != bxVec.end(); ++bxItr) {
    
    TriggerResponse * triggerResponse = new TriggerResponse();
    
    for( int k=0; k < m_maxWheels; ++k )
    {
      
      if ( m_Wheels[k].process( (*bxItr) , (*linkboardin) ) ) {
        
        m_Wheels[k].createWheelMap();
        
        m_Wheels[k].retrieveWheelMap( (m_ttuin[k]) );
        
        //.. execute selected logic at Ttu level
        m_ttuconf->m_ttulogic->run( (m_ttuin[k]) );
        
        //... and produce a Wheel level trigger
        trg = m_ttuconf->m_ttulogic->isTriggered();
        
        m_trigger.set(k,trg);
        
        if( m_debug ) std::cout << "TTUEmulator::processTtu ttuid: " << m_id 
                                << " bx: "          << (*bxItr)
                                << " wheel: "       << m_Wheels[k].getid()
                                << " response: "    << trg << std::endl;
        
      }

      
    }

    
    triggerResponse->setTriggerBits( (*bxItr) , m_trigger );
    m_triggerBxVec.push_back( triggerResponse );
    m_triggerBx[ (*bxItr) ] = m_trigger;
    
  }
  
  
  if( m_debug ) std::cout << "TTUEmulator::processTtu> size of trigger map " 
                          << m_triggerBx.size() << std::endl;
  
  
  if( m_debug ) std::cout << "TTUEmulator::processTtu> done with this TTU: " << m_id << std::endl;

  bxVec.clear();
    
}

void TTUEmulator::clearTriggerResponse()
{
  
  std::vector<TriggerResponse*>::iterator itr;
  for ( itr = m_triggerBxVec.begin(); itr != m_triggerBxVec.end(); ++itr)
    if ( (*itr) ) delete (*itr);
  m_triggerBxVec.clear();
  
}

//.................................................................

void TTUEmulator::printinfo() 
{
  
  std::cout << "TTUEmulator: " << m_id << '\n';
  for( int k=0; k < m_maxWheels; ++k ) 
    m_Wheels[k].printinfo();
  
}

