cbuffer PerFrameVariables : register(b0)
{
	float deltaTime;
	float elapsedTime;
}

cbuffer DeadListCount : register(b1)
{
	uint deadCount;
}

cbuffer EmitterParameters : register(b2)
{
	int emitterType;
	int numToEmitPerFrame;
	float particleLife;
	float startSize;
	float endSize;
	float pad6;
	float pad4;
	float pad5;
	float3 emitterPosition;
	float pad1;
	float3 emitterVelocity;
	float pad;
	float3 particlePosition;
	float pad2;
	float3 particleVelocity;
	float pad3;
	float4 startColour;
	float4 endColour;
	float4 currentColour;
	float currentSize;
}

cbuffer Variances : register(b3)
{
	float3 positionVariance;
	float pad7;
	float3 velocityVariance;
	float pad8;
	//float lifeVariance;
}

struct particles
{
	float3 position;
	float3 velocity;
	float life;
	float age;
	bool alive;
	float startSize;
	float endSize;
	float4 startColour;
	float4 endColour;
	float4 currentColour;
	float currentSize;
};

RWStructuredBuffer<particles> particleBuffer : register(u0);
ConsumeStructuredBuffer<uint> deadListToGetFrom	: register(u1);

//Textures filled with random numbers
Texture2D randomValues : register(t0);
Texture2D piRandomValues : register(t1);

SamplerState sampleState : register(s0);

[numthreads(512, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	if (DTid.x < deadCount && DTid.x < numToEmitPerFrame)
	{
		particles p = (particles)0;

		//Gets a value from the random texture
		float2 texPos = float2(DTid.x / 1024.0, elapsedTime);
		float3 posVarSeed = randomValues.SampleLevel(sampleState, texPos, 0).xyz;

		float2 texPos2 = float2((DTid.x) / 1024.0, elapsedTime);
		float3 velVarSeed = randomValues.SampleLevel(sampleState, texPos2, 0).xyz;

		float2 texPos3 = float2((DTid.x + 2) / 1024.0, elapsedTime);
		float lifeVarSeed = randomValues.SampleLevel(sampleState, texPos3, 0);

		//Gets a value to be used from the pi texture
		float2 texPos4 = float2((DTid.x) / 1024.0, elapsedTime);
		float3 piValues = piRandomValues.SampleLevel(sampleState, texPos4, 0).xyz;

		//Gives final random values to be used in the emission of the particles
		float3 posVar = float3((posVarSeed.x * positionVariance.x),
			(posVarSeed.y * positionVariance.y), 
			(posVarSeed.z * positionVariance.z));

		float3 velVar = float3((velVarSeed.x * velocityVariance.x),
			(velVarSeed.y * velocityVariance.y),
			(velVarSeed.z * velocityVariance.z));

		float lifeVar = lifeVarSeed * particleLife;

		//----------------------------Individual emitter code---------------------------------//

		//Omni Emitter
		if (emitterType == 0)
		{
			p.position.xyz = emitterPosition.xyz + particlePosition.xyz + posVar;
			p.velocity.xyz = emitterVelocity.xyz + particleVelocity.xyz + velVar;
		}

		//Volume Cube Emitter
		if (emitterType == 1)
		{
			p.position.xyz = emitterPosition.xyz + particlePosition.xyz + posVar;
			p.velocity.xyz = emitterVelocity.xyz + particleVelocity.xyz + velVar;
		}
		
		//Volume Sphere Emitter
		if (emitterType == 2)
		{
			float x = emitterPosition.x + (3.0f * cos(piValues.x) * cos(piValues.y));
			float y = emitterPosition.y + (3.0f * sin(piValues.y));
			float z = emitterPosition.z + (3.0f * sin(piValues.x) * cos(piValues.y));

			p.position.xyz = float3(x, y, z) + posVar;
			p.velocity.xyz = emitterVelocity.xyz + particleVelocity.xyz + velVar;
		}
		
		//Volume Torus Emitter
		if (emitterType == 3)
		{
			float x = (5.0f + 1.0f * cos(piValues.x)) * cos(piValues.y);
			float y = 1.0f * sin(piValues.x);
			float z = (5.0f + 1.0f * cos(piValues.x)) * sin(piValues.y);

			p.position.xyz = emitterPosition.xyz + particlePosition.yxz + float3(x, y, z) + posVar;
			p.velocity.xyz = emitterVelocity.xyz + particleVelocity.xyz + velVar;
		}

		//Curve Emitter
		if (emitterType == 4)
		{
			float x = piValues.x;
			float y = 0.0f;
			float z = sin(-0.5f * x);
			
			p.position.xyz = emitterPosition.xyz + particlePosition.yxz + float3(x, y, z) + posVar;
			p.velocity.xyz = emitterVelocity.xyz + particleVelocity.xyz + velVar;
		}

		//------------------------------------------------------------------------------------//

		//Global particle variables that will set in the same way for all emitter types
		p.life = lifeVar;
		p.age = p.life;
		p.alive = true;

		p.startSize = startSize;
		p.endSize = endSize;
		p.startColour = startColour;
		p.endColour = endColour;
		p.currentColour = startColour;
		p.currentSize = startSize;

		uint index = deadListToGetFrom.Consume();

		particleBuffer[index] = p;
	}
}