#ifndef PS_TRACKER_H
#define PS_TRACKER_H

namespace PS{

/*!
  * basic control mechanism for tracking changes in data structures.
  */
class RevisionTracker
{
public:
    RevisionTracker() {m_version = 0;}

    void set(int version) {m_version = version;}
    int get() const {return m_version;}

    bool isChanged() const {return (m_version > 0);}

    //Returns previous value
    int bump()  { return m_version++;}
    void reset() { m_version = 0;}

private:
    int m_version;
};

}

#endif // PS_TRACKER_H
