// This file was automatically generated by protocompiler
// DO NOT EDIT!
// Compiled from stdin

#import <TargetConditionals.h>
#if !TARGET_OS_BRIDGE

#import "AWDKeychainSOSKeychainBackupFailed.h"
#import <ProtocolBuffer/PBConstants.h>
#import <ProtocolBuffer/PBHashUtil.h>
#import <ProtocolBuffer/PBDataReader.h>

#if !__has_feature(objc_arc)
# error This generated file depends on ARC but it is not enabled; turn on ARC, or use 'objc_use_arc' option to generate non-ARC code.
#endif

@implementation AWDKeychainSOSKeychainBackupFailed

@synthesize timestamp = _timestamp;
- (void)setTimestamp:(uint64_t)v
{
    _has.timestamp = YES;
    _timestamp = v;
}
- (void)setHasTimestamp:(BOOL)f
{
    _has.timestamp = f;
}
- (BOOL)hasTimestamp
{
    return _has.timestamp;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"%@ %@", [super description], [self dictionaryRepresentation]];
}

- (NSDictionary *)dictionaryRepresentation
{
    NSMutableDictionary *dict = [NSMutableDictionary dictionary];
    if (self->_has.timestamp)
    {
        [dict setObject:[NSNumber numberWithUnsignedLongLong:self->_timestamp] forKey:@"timestamp"];
    }
    return dict;
}

BOOL AWDKeychainSOSKeychainBackupFailedReadFrom(__unsafe_unretained AWDKeychainSOSKeychainBackupFailed *self, __unsafe_unretained PBDataReader *reader) {
    while (PBReaderHasMoreData(reader)) {
        uint32_t tag = 0;
        uint8_t aType = 0;

        PBReaderReadTag32AndType(reader, &tag, &aType);

        if (PBReaderHasError(reader))
            break;

        if (aType == TYPE_END_GROUP) {
            break;
        }

        switch (tag) {

            case 1 /* timestamp */:
            {
                self->_has.timestamp = YES;
                self->_timestamp = PBReaderReadUint64(reader);
            }
            break;
            default:
                if (!PBReaderSkipValueWithTag(reader, tag, aType))
                    return NO;
                break;
        }
    }
    return !PBReaderHasError(reader);
}

- (BOOL)readFrom:(PBDataReader *)reader
{
    return AWDKeychainSOSKeychainBackupFailedReadFrom(self, reader);
}
- (void)writeTo:(PBDataWriter *)writer
{
    /* timestamp */
    {
        if (self->_has.timestamp)
        {
            PBDataWriterWriteUint64Field(writer, self->_timestamp, 1);
        }
    }
}

- (void)copyTo:(AWDKeychainSOSKeychainBackupFailed *)other
{
    if (self->_has.timestamp)
    {
        other->_timestamp = _timestamp;
        other->_has.timestamp = YES;
    }
}

- (id)copyWithZone:(NSZone *)zone
{
    AWDKeychainSOSKeychainBackupFailed *copy = [[[self class] allocWithZone:zone] init];
    if (self->_has.timestamp)
    {
        copy->_timestamp = _timestamp;
        copy->_has.timestamp = YES;
    }
    return copy;
}

- (BOOL)isEqual:(id)object
{
    AWDKeychainSOSKeychainBackupFailed *other = (AWDKeychainSOSKeychainBackupFailed *)object;
    return [other isMemberOfClass:[self class]]
    &&
    ((self->_has.timestamp && other->_has.timestamp && self->_timestamp == other->_timestamp) || (!self->_has.timestamp && !other->_has.timestamp))
    ;
}

- (NSUInteger)hash
{
    return 0
    ^
    (self->_has.timestamp ? PBHashInt((NSUInteger)self->_timestamp) : 0)
    ;
}

- (void)mergeFrom:(AWDKeychainSOSKeychainBackupFailed *)other
{
    if (other->_has.timestamp)
    {
        self->_timestamp = other->_timestamp;
        self->_has.timestamp = YES;
    }
}

@end
#endif
