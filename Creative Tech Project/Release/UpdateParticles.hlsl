//cbuffer PerFrameVariables : register(b0)
//{
//	float deltaTime;
//}
//
//struct particles
//{
//	float3 position;
//	float3 velocity;
//	float life;
//	float age;
//	bool alive;
//	float startSize;
//	float endSize;
//	float4 startColour;
//	float4 endColour;
//};
//
//RWStructuredBuffer<particles>	particleBuffer		: register(u0);
//
//AppendStructuredBuffer<uint>			g_DeadListToAddTo		: register(u1);
//
//[numthreads(256, 1, 1)]
//void main(uint3 DTid : SV_DispatchThreadID)
//{
//	particles p = (particles)0;
//
//	p = particleBuffer[DTid.x];
//	
//	if (p.age > 0.0f)
//	{
//		float3 newPos = 0;
//		float3 newVel = 0;
//
//		newPos.xyz = p.position.xyz;
//		newVel.xyz = p.velocity.xyz;
//		
//		//newVel.y = p.velocity.y + gravity;
//		
//		float pos = -0.2;
//
//		////////Radial field
//		//if ((p.position.x - -0.2)*(p.position.x - pos) + (p.position.y - 0.0f)*(p.position.y - 0.0f) + (p.position.z - 0.0f)*(p.position.z - 0.0f) < 0.2f * 0.2f)
//		//{
//		//	float forceMagX = (pos - p.position.x);
//		//	//if (forceMagX < 0)
//		//	//	forceMagX = abs(forceMagX);
//
//		//	float forceMagY = (0.0f - p.position.y);
//		//	//if (forceMagY < 0)
//		//		//forceMagY = abs(forceMagY);
//
//		//	float forceMagZ = (0.0f - p.position.z);
//		//	//if (forceMagZ < 0)
//		//	//	forceMagZ = abs(forceMagZ);
//
//		//	float forceX = (pos - p.position.x);
//		//	float forceY = (0.0f - p.position.y);
//		//	float forceZ = (0.0f - p.position.z);
//
//		//	newPos.xyz = p.position.xyz + newVel.xyz * deltaTime;
//		//	newVel.xyz = float3(p.velocity.x - forceX, p.velocity.y - forceY, p.velocity.z - forceZ);
//		//	p.startColour.z = 1.0f;
//		//}
//
//		if ((p.position.x - 0.0f)*(p.position.x - 0.0f) + (p.position.z - 0.0f)*(p.position.z - 0.0f) < 0.3f * 0.3f)
//		{
//			float dx = p.position.x - 0.0f;
//			float dy = p.position.z - 0.0f;
//
//			float vx = -dy*3.0f;
//			float vy = dx*3.0f;
//
//			float factor = 1.0f / (1.0f + (dx*dx + dy*dy) / 0.1f);
//
//			float xx = (vx - newVel.x)*factor;
//			float zz = (vy - newVel.z)*factor;
//
//			//float3 r = p.position - 0.0f;
//			//float3 v = cross(5.0f, 0.4f);
//
//			newVel.xyz = float3(p.velocity.x + xx, p.velocity.y, p.velocity.z + zz);
//		}
//
//		//if ((p.position.x - 0.0f)*(p.position.x - 0.0f) + (p.position.y - 0.0f)*(p.position.y - 0.0f) + (p.position.z - 0.0f)*(p.position.z - 0.0f) < 0.3f * 0.3f)
//		//{
//		//	p.startSize += 0.005f;
//		//	float pi = 3.14159265359;
//
//		//	float forceMagX = (0.0f + (0.5f*sin(p.startSize)));
//		//	float forceMagY = (0.0f + (0.5f*cos(p.startSize)));
//		//	float forceMagZ = (0.0f - p.position.z) / 0.6f;
//
//		//	float forceX = (0.0f - p.position.x);
//		//	float forceY = (0.0f - p.position.y);
//		//	float forceZ = (0.0f - p.position.z);
//
//		//	//newPos.xyz = p.position.xyz + newVel.xyz * deltaTime;
//		//	newVel.xyz = float3(forceMagX, forceMagY, 0.0f);
//		//	//p.startColour.z = 1.0f;
//		//}
//
//		//float forceX = 0.0f + cos(p.age) * 0.2f;
//		//float forceY = 0.0f + sin(p.age) * 0.2f;
//		//float forceZ = 0.0f + cos(p.age) * 0.2f;
//
//		//newPos.xyz = p.position.xyz + newVel.xyz * deltaTime;
//		//newVel.xyz = float3(p.velocity.x + forceX, p.velocity.y, p.velocity.z + forceZ);
//
//		newPos.xyz = p.position.xyz + newVel.xyz * deltaTime;
//		//newVel.xyz = float3(p.velocity.x, p.velocity.y, p.velocity.z);
//
//		p.position.xyz = newPos.xyz;
//		p.velocity.xyz = newVel.xyz;
//
//		float3 tempCurrentPos;
//
//		p.age -= deltaTime;
//
//		float colourDiffX = p.startColour.x + (p.startColour.x + 1.0f * (p.endColour.x - p.startColour.x));
//		float colourDiffY = p.startColour.y + (p.startColour.y + 1.0f * (p.endColour.y - p.startColour.y));
//		float colourDiffZ = p.startColour.z + (p.startColour.z + 1.0f * (p.endColour.z - p.startColour.z));
//		///float colourDiffA = p.startColour.w + (p.startColour.w + 1.0f * (p.endColour.w - p.startColour.w));
//		
//		float perFrameX = (colourDiffX / p.life) * deltaTime;
//		float perFrameY = (colourDiffY / p.life) * deltaTime;
//		float perFrameZ = (colourDiffZ / p.life) * deltaTime;
//		//float perFrameA = (colourDiffA / p.life) * deltaTime;
//
//		p.startColour.x = lerp(p.startColour.x, p.endColour.x, perFrameX);
//		p.startColour.y = lerp(p.startColour.y, p.endColour.y, perFrameY);
//		p.startColour.z = lerp(p.startColour.z, p.endColour.z, perFrameZ);
//		//p.startColour.w = lerp(p.startColour.w, p.endColour.w, perFrameA);
//	}
//
//	if (p.age <= 0.0f && p.alive == true)
//	{
//		g_DeadListToAddTo.Append(DTid.x);
//		p.life = -1.0f;
//		p.alive = false;
//		p.position.xyz = 9999.0f;
//		p.velocity.xyz = 0.0f;
//		p.startSize = 1.0f;
//		p.startColour = float4(1.0f, 1.0f, 1.0f, 1.0f);
//		p.endColour = float4(0.0f, 0.0f, 0.0f, 1.0f);
//	}
//
//	particleBuffer[DTid.x] = p;
//}

//cbuffer PerFrameVariables : register(b0)
//{
//	float deltaTime;
//}
//cbuffer EmitterParameters : register(b1){ int emitterType;	int numToEmitPerFrame;	float particleLife;	float startSize;	float endSize;	float pad6;	float pad4;	float pad5;	float3 emitterPosition;	float pad1;	float3 emitterVelocity;	float pad;	float3 particlePosition;	float pad2;	float3 particleVelocity;	float pad3;	float4 startColour;	float4 endColour; }struct particles
//{
//	float3 position;
//	float3 velocity;
//	float life;
//	float age;
//	bool alive;
//	float startSize;
//	float endSize;
//	float4 startColour;
//	float4 endColour;
//}; RWStructuredBuffer<particles>	particleBuffer		: register(u0);
//AppendStructuredBuffer<uint>			g_DeadListToAddTo		: register(u1);
//[numthreads(256, 1, 1)]
//void main(uint3 DTid : SV_DispatchThreadID)
//{
//	particles p = (particles)0;
//	p = particleBuffer[DTid.x];
//	if (p.age > 0.0f)
//	{
//		float3 newPos = 0;
//			float3 newVel = 0;
//			float forceX = 0;
//		float forceY = 0;
//		float forceZ = 0;
//		newPos.xyz = p.position.xyz;
//		newVel.xyz = p.velocity.xyz;
//		float fScaledLife = 1.0 - saturate(p.age / p.life);		float4 color0 = startColour;		float4 color1 = endColour;		p.startColour = lerp(color0, color1, saturate(fScaledLife)).xyzw;		float size0 = startSize;		float size1 = endSize;		p.startSize = lerp(size0, size1, saturate(fScaledLife));       p.age -= deltaTime;
//		newPos.xyz = p.position.xyz + newVel.xyz * deltaTime;
//		newVel.xyz = float3(p.velocity.x, p.velocity.y, p.velocity.z);
//		p.position.xyz = newPos.xyz;
//		p.velocity.xyz = newVel.xyz;
//	}
//	if (p.age <= 0.0f && p.alive == true)
//	{
//		g_DeadListToAddTo.Append(DTid.x);
//		p.life = -1.0f;
//		p.alive = false;
//		p.position.xyz = 9999.0f;
//		p.velocity.xyz = 0.0f;
//		p.startSize = 1.0f;
//		p.startColour = float4(1.0f, 0.0f, 1.0f, 1.0f);
//		p.endColour = float4(0.0f, 0.0f, 0.0f, 1.0f);
//	}
//	particleBuffer[DTid.x] = p;
//}
