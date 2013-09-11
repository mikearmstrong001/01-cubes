#include "anim.h"


float animSample( const Anim &anim, int meshIndex, int channelIndex, float t, float def )
{
	if ( anim.looping )
	{
		//t = fmodf( fmodf( t, anim.time ) + anim.time, anim.time );
		t = fmodf( fmodf( t, anim.time ) + anim.time, anim.time ) / anim.time;
	} else
	{
		t = t / anim.time;
	}
	const AnimChannel &ch = anim.channels[meshIndex];
	const std::vector< AnimKey > &keys = ch.keys[channelIndex];
	if ( keys.size() == 0 )
	{
		return def;
	}
	unsigned int i = 0;
	while ( i < keys.size() && t > keys[i].time )
		i++;
	if ( i >= keys.size() )
	{
		return keys[i-1].value;
	}
	else
	if ( i == 0 )
	{
		return keys[0].value;
	}
	else
	{
		const AnimKey &k0 = keys[i-1];
		const AnimKey &k1 = keys[i];
		float lerp = (t - k0.time) / (k1.time - k0.time);
		return (1.f - lerp) * k0.value + (lerp) * k1.value;
	}
}

int   animTags( const Anim &anim, float prevT, float curT )
{
	if ( anim.looping )
	{
		//t = fmodf( fmodf( t, anim.time ) + anim.time, anim.time );
		curT = fmodf( fmodf( curT, anim.time ) + anim.time, anim.time ) / anim.time;
		prevT = fmodf( fmodf( prevT, anim.time ) + anim.time, anim.time ) / anim.time;
	} else
	{
		curT = curT / anim.time;
		prevT = prevT / anim.time;
	}
	int out = 0;
	for (unsigned int i=0; i<anim.tags.size(); i++)
	{
		const AnimTag &tag = anim.tags[i];
		if ( prevT < curT && prevT < tag.time && curT >= tag.time )
			out |= (1<<tag.id);
		else if ( prevT > curT && prevT > tag.time && curT >= tag.time )
			out |= (1<<tag.id);
	}
	return out;
}


