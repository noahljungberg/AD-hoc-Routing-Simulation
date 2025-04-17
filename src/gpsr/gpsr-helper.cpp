#include "gpsr/gpsr-helper.h"
#include "gpsr/gpsr.h"
#include "ns3/node-list.h"
#include "ns3/names.h"
#include "ns3/ipv4-list-routing.h"
#include "ns3/node-container.h"
#include "ns3/callback.h"
#include "ns3/udp-l4-protocol.h"

namespace ns3 {

    GpsrHelper::GpsrHelper() :
      Ipv4RoutingHelper()
    {
        m_agentFactory.SetTypeId("Gpsr");
    }

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

        for (NodeContainer::Iterator i = nodes.Begin(); i != nodes.End(); ++i)
        {
            Ptr<Node> node = *i;

            // Get L4 protocol - this is generally UDP
            Ptr<UdpL4Protocol> udp = node->GetObject<UdpL4Protocol>();

            // Get GPSR instance
            Ptr<Gpsr> gpsr = node->GetObject<Gpsr>();

            // Connect GPSR to UDP
            if (gpsr && udp) {
                gpsr->SetDownTarget(udp->GetDownTarget());
                udp->SetDownTarget(MakeCallback(&Gpsr::AddHeaders, gpsr));
            }
        }
    }

} // namespace ns3