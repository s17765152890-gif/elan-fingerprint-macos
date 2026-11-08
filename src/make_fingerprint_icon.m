#import <Cocoa/Cocoa.h>
static void ridge(CGFloat x,CGFloat y,CGFloat w,CGFloat h,CGFloat tail){
 NSBezierPath*p=[NSBezierPath bezierPath];[p moveToPoint:NSMakePoint(x,y+tail)];
 [p curveToPoint:NSMakePoint(x,y+h*.58) controlPoint1:NSMakePoint(x+10,y+tail+65) controlPoint2:NSMakePoint(x,y+h*.40)];
 [p curveToPoint:NSMakePoint(x+w*.5,y+h) controlPoint1:NSMakePoint(x,y+h*.83) controlPoint2:NSMakePoint(x+w*.22,y+h)];
 [p curveToPoint:NSMakePoint(x+w,y+h*.58) controlPoint1:NSMakePoint(x+w*.78,y+h) controlPoint2:NSMakePoint(x+w,y+h*.83)];
 [p curveToPoint:NSMakePoint(x+w*.70,y-18) controlPoint1:NSMakePoint(x+w,y+h*.25) controlPoint2:NSMakePoint(x+w*.90,y+38)];
 p.lineWidth=24;p.lineCapStyle=NSLineCapStyleRound;[p stroke];
}
int main(int argc,char**argv){@autoreleasepool{if(argc!=2)return 1;NSImage*im=[[NSImage alloc]initWithSize:NSMakeSize(1024,1024)];[im lockFocus];
 [[NSColor colorWithCalibratedRed:.065 green:.11 blue:.19 alpha:1]setFill];[[NSBezierPath bezierPathWithRoundedRect:NSMakeRect(0,0,1024,1024) xRadius:220 yRadius:220]fill];
 [[NSColor colorWithCalibratedRed:.20 green:.88 blue:.70 alpha:1]setStroke];ridge(210,240,604,570,100);ridge(270,260,484,490,64);ridge(330,285,364,405,40);
 [[NSColor colorWithCalibratedRed:.77 green:1 blue:.93 alpha:1]setStroke];ridge(390,312,244,316,30);
 NSBezierPath*p=[NSBezierPath bezierPath];[p moveToPoint:NSMakePoint(456,325)];[p curveToPoint:NSMakePoint(512,565) controlPoint1:NSMakePoint(480,400) controlPoint2:NSMakePoint(440,565)];[p curveToPoint:NSMakePoint(552,390) controlPoint1:NSMakePoint(580,565) controlPoint2:NSMakePoint(575,440)];p.lineWidth=24;p.lineCapStyle=NSLineCapStyleRound;[p stroke];
 [im unlockFocus];NSBitmapImageRep*b=[NSBitmapImageRep imageRepWithData:im.TIFFRepresentation];NSData*d=[b representationUsingType:NSBitmapImageFileTypePNG properties:@{}];return [d writeToFile:[NSString stringWithUTF8String:argv[1]] atomically:YES]?0:2;}}
