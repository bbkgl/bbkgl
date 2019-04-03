//
// Created by bbkgl on 19-3-27.
//

#ifndef BBKGL_TIMESTAMP_H
#define BBKGL_TIMESTAMP_H


#include <stdint.h>
#include <string>

class Timestamp
    // public boost::less_than_comparable<Timestamp>,
    // public boost::equality_comparable<Timestamp>
{
public:
    ///
    /// Constucts an invalid Timestamp.
    ///
    Timestamp();

    ///
    /// Constucts a Timestamp at specific time
    ///
    /// @param microSecondsSinceEpoch
    explicit Timestamp(int64_t microSecondsSinceEpoch);

    void swap(Timestamp& that)
    {
        std::swap(microSecondsSinceEpoch_, that.microSecondsSinceEpoch_);
    }

    // default copy/assignment/dtor are Okay

    std::string toString() const;
    std::string toFormattedString() const;

    bool valid() const { return microSecondsSinceEpoch_ > 0; }

    // for internal usage.
    int64_t microSecondsSinceEpoch() const { return microSecondsSinceEpoch_; }

    ///
    /// Get time of now.
    ///
    static Timestamp now();

    ///
    /// Get an invalid time.
    ///
    static Timestamp invalid();

    static const int kMicroSecondsPerSecond = 1000 * 1000;

private:
    int64_t microSecondsSinceEpoch_;
};

inline bool operator<(Timestamp lhs, Timestamp rhs)
{
    return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
}

inline bool operator==(Timestamp lhs, Timestamp rhs)
{
    return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
}

inline double timeDifference(Timestamp high, Timestamp low)
{
    int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
    return static_cast<double>(diff) / Timestamp::kMicroSecondsPerSecond;
}

inline Timestamp addTime(Timestamp timestamp, double seconds)
{
    int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
    return Timestamp(timestamp.microSecondsSinceEpoch() + delta);
}


#endif //BBKGL_TIMESTAMP_H
