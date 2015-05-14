#include <avrSystemMarker.h>

avrSystemMarker::avrSystemMarker()
{
   this->projection = new avrMatrix3x4();
}

avrSystemMarker::~avrSystemMarker(){
    //destructor
}


void avrSystemMarker::addPattern(avrPattern& patt){
   this->patts.push_back(&patt);
}

int avrSystemMarker::sizePatts() const{
   return (int)this->patts.size();
}

avrPattern& avrSystemMarker::getPatt(int index) const{
   return *(this->patts.at(index));
}

avrMatrix3x4& avrSystemMarker::getProjection() const{
   return *(this->projection);
}
