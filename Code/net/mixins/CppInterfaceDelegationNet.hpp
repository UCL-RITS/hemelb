#ifndef HEMELB_NET_MIXINS_CPPINTERFACEDELEGATIONNET_HPP
#define HEMELB_NET_MIXINS_CPPINTERFACEDELEGATIONNET_HPP
namespace hemelb
{
  namespace net
  {
    template<class BaseNet> class CppInterfaceDelegationNet : public virtual BaseNet
    {
      public:
        template<class T>
        void RequestSendV(std::vector<T> &payload, proc_t toRank)
        {
          BaseNet::RequestSend(&payload[0], payload.size(), toRank);
        }

        template<class T>
        void RequestSendR(T& value, proc_t toRank)
        {
          BaseNet::RequestSend(&value, 1, toRank);
        }

        template<class T>
        void RequestReceiveR(T& value, proc_t fromRank)
        {
          BaseNet::RequestReceive(&value, 1, fromRank);
        }

        template<class T>
        void RequestReceiveV(std::vector<T> &payload, proc_t toRank)
        {
          BaseNet::RequestReceive(&payload[0], payload.size(), toRank);
        }

        template<class T>
        void RequestGatherVReceive(std::vector<std::vector<T> > &buffer)
        {
          std::vector<int> & displacements=this->GetDisplacementsBuffer();
          std::vector<int> &counts=this->GetCountsBuffer();

          log::Logger::Log<log::Debug, log::OnePerCore>("Request receiving GatherV getting %i",
                                                        buffer[0].size());

          for (typename std::vector<std::vector<T> >::iterator buffer_iterator = buffer.begin();
              buffer_iterator != buffer.end(); buffer_iterator++)
          {
            // Ensure each vector has some underlying array, even if it's unused.
            buffer_iterator->reserve(1);
            displacements.push_back(&buffer_iterator->front() - &buffer.front().front());
            log::Logger::Log<log::Debug, log::OnePerCore>("Getting %i from this piece",buffer_iterator->size());
            counts.push_back(buffer_iterator->size());
          }
          BaseNet::RequestGatherVReceive(&buffer.front().front(), &displacements.front(), &counts.front());
        }

        template<class T>
        void RequestGatherReceive(std::vector<T> &buffer)
        {
          // Ensure vector has some underlying array, even if it's unused.
          buffer.reserve(1);
          BaseNet::RequestGatherReceive(&buffer.front());
        }

        template<class T>
        void RequestGatherSend(T& value, proc_t toRank)
        {
          BaseNet::RequestGatherSend(&value, toRank);
        }

        template<class T>
        void RequestGatherVSend(std::vector<T> &payload, proc_t toRank)
        {
          BaseNet::RequestGatherVSend(&payload.front(), payload.size(), toRank);
        }
    };
  }
}
#endif
