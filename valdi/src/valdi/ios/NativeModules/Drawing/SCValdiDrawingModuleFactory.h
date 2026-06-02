//
//  SCValdiDrawingModuleFactory.h
//  valdi-ios
//
//  Created by Simon Corsin on 8/26/19.
//

#import "valdi_core/SCNValdiCoreModuleFactory.h"
#import <Foundation/Foundation.h>
#import <SCCDrawingTypes/SCCDrawingTypes.h>

NS_ASSUME_NONNULL_BEGIN

@class SCValdiFontManager;

@interface SCValdiDrawingModuleFactory : NSObject <SCNValdiCoreModuleFactory, SCValdiDrawingModule>

- (instancetype)initWithFontManager:(SCValdiFontManager*)fontManager;

@end

NS_ASSUME_NONNULL_END
