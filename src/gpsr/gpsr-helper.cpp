#include "gpsr/gpsr-helper.hpp"
#include "gpsr/gpsr.h"
#include "ns3/node-list.h"
#include "ns3/names.h"
#include "ns3/ipv4-list-routing.h"
#include "ns3/node-container.h"
#include "ns3/callback.h"
#include "ns3/udp-l4-protocol.h"
#include "ns3/log.h"
NS_LOG_COMPONENT_DEFINE("GpsrHelper");  // Define the log component

namespace ns3 {

    GpsrHelper::GpsrHelper() :
      Ipv4RoutingHelper()
    {
  m_agentFactory.SetTypeId("ns3::Gpsr");    }

    GpsrHelper*
    GpsrHelper::Copy() const
    {
        return new GpsrHelper(*this);
    }

    Ptr<Ipv4RoutingProtocol>
    GpsrHelper::Create(Ptr<Node> node) const
    {
        Ptr<Gpsr> gpsr = m_agentFactory.Create<Gpsr>();
        node->AggregateObject(gpsr);
        return gpsr;
    }

    void
    GpsrHelper::Set(std::string name, const AttributeValue &value)
    {
        m_agentFactory.Set(name, value);
    }

    void
GpsrHelper::Install() const
    {
        NodeContainer nodes = NodeContainer::GetGlobal();
        NS_LOG_INFO("Installing GPSR on " << nodes.GetN() << " nodes");

        for (NodeContainer::Iterator i = nodes.Begin(); i != nodes.End(); ++i)
        {
            Ptr<Node> node = *i;
            NS_LOG_INFO("Installing GPSR on node " << node->GetId());

            // Connect GPSR to IP layer
            Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
            if (!ipv4) {
                NS_LOG_ERROR("No IPv4 object found on node " << node->GetId());
                continue;
            }

            // Make sure this call is included!
            Ptr<Ipv4ListRouting> listRouting = DynamicCast<Ipv4ListRouting>(ipv4->GetRoutingProtocol());
            if (!listRouting) {
                NS_LOG_ERROR("No Ipv4ListRouting found on node " << node->GetId());
                continue;
            }

            // Get GPSR instance and ensure it's at the top of the list
            Ptr<Gpsr> gpsr = node->GetObject<Gpsr>();
            if (gpsr) {
                listRouting->AddRoutingProtocol(gpsr, 10); // Higher priority number
                NS_LOG_INFO("Successfully connected GPSR to IPv4 on node " << node->GetId());
            } else {
                NS_LOG_ERROR("No GPSR object found on node " << node->GetId());
            }
        }
    }

} // namespace ns3